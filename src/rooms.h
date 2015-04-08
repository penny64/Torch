#include "items.h"

#define MAX_ROOMS 6
#define MAX_CONNECTED_ROOMS 5

#ifndef ROOMS_H
#define ROOMS_H


typedef struct room room;

struct room {
	int id, centerX, centerY, size, numberOfDoorPositions, numberOfConnectedRooms, numberOfOccupiedSpawnPositions;
	int x, y, width, height;
	int *connectedRooms, *spawnPositions, **positionList, **doorPositions;
	unsigned int flags;
	struct room *next, *prev;
};

typedef struct roomProto roomProto;

struct roomProto {
	int id, x, y, width, height, size;
	roomProto *parent;
};

#endif

void placeItemInRoom(room*, item*);
void createAndPlaceItemInRoom(room*, void(*)(int, int));
void deleteAllRooms(void);
void addRoomDoorPosition(room*, int, int);
void claimSpawnPositionInRoom(room*, int, int);
void getNewSpawnPosition(room*, int[]);
void connectRooms(room*, room*);
int isRoomConnectedTo(room*, room*);
int isRoomConnectedToId(room*, int);
int isPositionInRoom(room*, int, int);

room *createRoom(roomProto*, unsigned int);
room *getRooms(void);
room *getRoomViaId(int);
room *getRoomWithFlags(unsigned int);

roomProto *createProtoRoom(int, int, int, int, roomProto*);
roomProto *splitProtoRoom(roomProto*, int);