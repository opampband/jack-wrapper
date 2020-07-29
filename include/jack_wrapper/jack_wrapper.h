#include <cstring>
#include <iostream>

#include <jack/jack.h>

/**
 * Wrapper around a Jack client.
 */
class JackClient {
public:
  JackClient(const char *clientName, JackProcessCallback processCallback,
             jack_port_t *&inputPort, jack_port_t *&outputPort)
      : clientName(clientName), processCallback(processCallback),
        inputPort(inputPort), outputPort(outputPort) {
    serverName = nullptr;
    options = JackNullOption;

    // Create a Jack client and connect to the Jack server but do not start
    // processing
    open();
  };

  ~JackClient() {
    close();
  }

  /**
   * Opens connection with the Jack server.
   * Also registers callback, ports, etc.
   */
  void open() {
    client = jack_client_open(clientName, options, &status, serverName);

    if (!client) {
      std::cerr << "jack_client_open() failed, status = 0x" << status << "\n";
      if (status & JackServerFailed) {
        std::cerr << "Unable to connect to JACK server\n";
      }
      exit(1);
    }

    if (status & JackServerStarted) {
      std::cout << "JACK server started\n";
    }

    // If clientName is not unique JACK will assign a unique name.
    // Get this name.
    if (status & JackNameNotUnique) {
      clientName = jack_get_client_name(client);
      std::cerr << "Unique name assigned: " << clientName << "\n";
    }

    // Register process callback and JACK shutdown callback
    // Do some hacky stuff to be able to register non-static functions.
    jack_set_process_callback(client, processCallback, 0);
    jack_on_shutdown(client, jackShutdown, 0);

    // Print current sample rate
    sampleRate = jack_get_sample_rate(client);
    std::cout << "Engine sample rate: " << sampleRate << "\n";

    // Print current buffer size
    // TODO implement a callback to handle buffer size changes
    bufferSize = jack_get_buffer_size(client);
    std::cout << "Engine buffer size: " << bufferSize << "\n";

    // Register input and output ports
    inputPort = jack_port_register(client, "input", JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
    outputPort = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

    if (!inputPort || !outputPort) {
      std::cerr << "No more JACK ports available!\n";
      exit(1);
    }
  }

  /**
   * Tells JACK server we are ready to start running process callback.
   */
  void run() {
    // Tell JACK server to start running callback.
    if (jack_activate (client)) {
      std::cerr << "Cannot activate client!\n";
      exit (1);
    }

    // Connect the ports.
    // This can only be done after the client is running.

    // Look for all physical capture ports.
    ports = jack_get_ports(client, NULL, NULL,
                           JackPortIsPhysical | JackPortIsOutput);
    if (ports == NULL) {
      std::cerr <<  "no physical capture ports\n";
      exit (1);
    }

    // Connect our inputPort to the first capture port found.
    if (jack_connect(client, ports[0], jack_port_name(inputPort))) {
      std::cerr <<  "cannot connect input ports\n";
    }

    free (ports);

    // Look for all physical playback ports
    ports = jack_get_ports (client, NULL, NULL,
                            JackPortIsPhysical|JackPortIsInput);
    if (ports == NULL) {
      std::cerr << "no physical playback ports\n";
      exit (1);
    }

    // Connect our outputPort to the first playback port found.
    if (jack_connect(client, jack_port_name(outputPort), ports[0])) {
      fprintf(stderr, "cannot connect output ports\n");
    }
    std::cout << "connected to playback port: " << ports[0] << "\n";

    free(ports);
  }

  /**
   * Closes JACK client.
   */
  void close() {
    jack_client_close(client);
  }

  std::size_t getBufferSize() {
    return bufferSize;
  }

  std::size_t getSampleRate() {
    return sampleRate;
  }


private:
  const char *clientName;

  // Callback function to be invoked when processing a buffer.
  JackProcessCallback processCallback;

  jack_port_t *&inputPort;
  jack_port_t *&outputPort;
  jack_client_t *client;
  const char **ports;
  const char *serverName;
  jack_options_t options;
  jack_status_t status;

  std::size_t bufferSize;
  std::size_t sampleRate;

  /**
   * JACK calls this if server shuts down or server disconnects client.
   */
  static void jackShutdown(void *arg) {
    exit(1);
  }
};

