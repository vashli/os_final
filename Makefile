OBJ = net_raid_client net_raid_server

all: net_raid_client net_raid_server

net_raid_client: net_raid_client.c
	gcc -o net_raid_client net_raid_client.c `pkg-config fuse --cflags --libs`
net_raid_server: net_raid_server.c
	gcc -o net_raid_server net_raid_server.c

clean :
	rm -f $(OBJ)
