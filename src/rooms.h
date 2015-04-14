#include "items.h"

#define MAX_ROOMS 6
#define MAX_CONNECTED_ROOMS 5
#define MAX_ROOMS_IN_GROUP 8

#ifndef ROOMS_H
#define ROOMS_H


enum {
	IS_PROTO_START = 0x01 << 0,
	IS_PROTO_EXIT = 0x01 << 1,
	IS_PROTO_MAIN_PATH = 0x01 << 2,
	IS_PROTO_SPECIAL_ROOM = 0x01 << 3,
} protoRoom_t;

enum {
	IS_TREASURE_ROOM = 0x01 << 0,
	IS_TORCH_ROOM = 0x01 << 1,
	IS_EXIT_ROOM = 0x01 << 2,
	NEEDS_DOORS = 0x01 << 3,
	IS_RARE_SPAWN = 0x01 << 4,
	IS_LAVA_ROOM = 0x01 << 5,
	IS_START_ROOM = 0x01 << 6,
	IS_PUZZLE_ROOM = 0x01 << 7,
	IS_FURNACE_ROOM = 0x01 << 8,
	IS_MAIN_PATH = 0x01 << 9,
	IS_SPECIAL_ROOM = 0x01 << 10,
} roomFlag_t;

typedef struct room room;
typedef struct roomProto roomProto;
typedef struct roomGroup roomGroup;

struct room {
	int id, centerX, centerY, size, numberOfDoorPositions, numberOfCombinedRooms, numberOfConnectedRooms, numberOfNeighborRooms, numberOfOccupiedSpawnPositions;
	int x, y, width, height, wasCombined;
	int *connectedRoomIds, *combinedRoomIds,  *neighborRoomIds, *spawnPositions, **positionList, **doorPositions;
	unsigned int flags;
	room *next, *prev;
	roomGroup *group;
};

struct roomProto {
	int id, x, y, width, height, size, build, timesSplit, merged;
	int numberOfGroupNeighbors;
	float cost;
	unsigned int flags; // :)
	roomGroup *group;
	roomProto **groupNeighbors;
};

struct roomGroup {
	int numberOfRooms, numberOfProtoRooms;
	roomProto **roomProtos;
	room **rooms;
};

#endif

void placeItemInRoom(room*, item*);
void createAndPlaceItemInRoom(room*, void(*)(int, int));
void deleteAllRooms(void);
void addRoomDoorPosition(room*, int, int);
void claimSpawnPositionInRoom(room*, int, int);
void getNewSpawnPosition(room*, int[]);
void addNeighbor(room*, room*);
void connectRooms(room*, room*);
void addProtoToRoomGroup(roomGroup*, roomProto*);
void addRoomToRoomGroup(roomGroup*, room*);
void addProtoAsGroupNeighbor(roomProto*, roomProto*);
void mergeProtoRooms(roomProto*, roomProto*);
int isNeighborWith(room *, room*);
int isNeighborWithId(room*, int);
int isPotentialCombinedRoom(room*, room*);
int isConnectedWith(room*, room*);
int isPositionInRoom(room*, int, int);
int isPositionSpawnable(room*, int, int);

room *createRoom(roomProto*, unsigned int);
room *getRooms(void);
room *getRoomViaId(int);
room *getRoomWithFlags(unsigned int);

roomProto *createProtoRoom(int, int, int, int, roomProto*);
roomProto *splitProtoRoom(roomProto*, int);

roomGroup *createRoomGroup();