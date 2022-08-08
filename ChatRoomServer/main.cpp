#include <enet/enet.h>
#include <iostream>

using namespace std;

ENetAddress address;
ENetHost* server;

bool CreateServer()
{
    cout << "Creating server..." << endl;

    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server != NULL;
}

void BroadcastMessage(string message)
{
    /* Create a reliable packet of size 7 containing "packet\0" */
    ENetPacket* packet = enet_packet_create(message.c_str(),
        strlen(message.c_str()) + 1,
        ENET_PACKET_FLAG_RELIABLE);

    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    enet_host_broadcast(server, 0, packet);

    /* One could just use enet_host_service() instead. */
    enet_host_flush(server);
}

int main(int argc, char** argv)
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }

    atexit(enet_deinitialize);

    if (!CreateServer())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        cout << "An error occurred while trying to create an ENet server host." << endl;
        ::exit(EXIT_FAILURE);
    }

    cout << "Server created..." << endl;

    while (true)
    {
        ENetEvent event;

        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                BroadcastMessage("System: A new user has connected.");

                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // relay the message to all users
                BroadcastMessage((char*)event.packet->data);

                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                BroadcastMessage("A user has disconnected.");

                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }

    if (server != NULL) enet_host_destroy(server);

    return EXIT_SUCCESS;
}