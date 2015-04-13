#include <stdio.h>
#include <assert.h>

#include "framework/display.h"
#include "framework/numbers.h"
#include "level.h"
#include "rooms.h"

room *ROOMS = NULL;
int NEXT_ROOM_ID = 0;


roomProto *createProtoRoom(int x, int y, int width, int height, roomProto *parentRoomProto) {
	roomProto *rm = calloc(1, sizeof(roomProto));

	rm->x = x;
	rm->y = y;
	rm->width = width;
	rm->height = height;
	rm->size = width * height;
	rm->parent = NULL; //ignore arg...
	rm->build = 0;
	rm->cost = 150;
	rm->flags = 0x0;
	rm->timesSplit = 1;
	rm->merged = 0;

	return rm;
}

void mergeProtoRooms(roomProto *srcRoom, roomProto *dstRoom) {
	srcRoom->merged = 1;
	dstRoom->merged = 1;

	dstRoom->parent = srcRoom;
	srcRoom->parent = dstRoom;
}

roomProto *splitProtoRoom(roomProto *parentRoomProto, int horizSplit) {
	int nx, ny, nWidth, nHeight, splitAmount;

	if (horizSplit) {
		splitAmount = (int)((parentRoomProto->height * getRandomFloat(.4, .6)) + .5);
		nHeight = parentRoomProto->height - splitAmount;
		parentRoomProto->height = splitAmount;

		nWidth = parentRoomProto->width;
		nx = parentRoomProto->x;
		ny = parentRoomProto->y + parentRoomProto->height;
	} else {
		splitAmount = (int)((parentRoomProto->width * getRandomFloat(.4, .6)) + .5);

		nWidth = parentRoomProto->width - splitAmount;
		parentRoomProto->width = splitAmount;
		nHeight = parentRoomProto->height;
		nx = parentRoomProto->x + parentRoomProto->width;
		ny = parentRoomProto->y;
	}

	if (nx + nWidth > WINDOW_WIDTH - 2) {
		nWidth -= (nx + nWidth) - (WINDOW_WIDTH - 2);
	}

	if (ny + nHeight > WINDOW_HEIGHT - 2) {
		nHeight -= (ny + nHeight) - (WINDOW_HEIGHT - 2);
	}

	parentRoomProto->size = parentRoomProto->width * parentRoomProto->height;
	parentRoomProto->timesSplit ++;

	return createProtoRoom(nx, ny, nWidth, nHeight, parentRoomProto);
}

room *createRoom(roomProto *prototypeRoom, unsigned int flags) {
	room *ptr, *rm = calloc(1, sizeof(room));
	int i, x, y, width, height;

	width = prototypeRoom->width - 1;
	height = prototypeRoom->height - 1;

	rm->id = NEXT_ROOM_ID;
	NEXT_ROOM_ID ++;

	rm->x = prototypeRoom->x;
	rm->y = prototypeRoom->y;

	if (prototypeRoom->parent) {
		if (prototypeRoom->parent->x < prototypeRoom->x) {
			rm->x --;
			width = (int) ((width * getRandomFloat(.6, .9)) + .5);
		} else if (prototypeRoom->parent->x > prototypeRoom->x) {
			width ++;
			rm->x += getRandomInt(1, (prototypeRoom->width - width) - 1);
		}

		if (prototypeRoom->parent->y < prototypeRoom->y) {
			rm->y --;
			height = (int) ((height * getRandomFloat(.6, .9)) + .5);
		} else if (prototypeRoom->parent->y > prototypeRoom->y) {
			height ++;
			rm->y += getRandomInt(1, (prototypeRoom->height - height) - 1);
		}
	} else {
		if (width > 9) {
			width = (int) ((width * getRandomFloat(.6, .85)) + .5);
		}

		if (height > 9) {
			height = (int) ((height * getRandomFloat(.6, .85)) + .5);
		}

		if (width < prototypeRoom->width) {
			rm->x += getRandomInt(0, (prototypeRoom->width - width) - 1);
		}

		if (height < prototypeRoom->height) {
			rm->y += getRandomInt(0, (prototypeRoom->height - height) - 1);
		}
	}

	rm->size = width * height;
	rm->width = width;
	rm->height = height;
	rm->centerX = rm->x + rm->width / 2;
	rm->centerY = rm->y + rm->height / 2;
	rm->numberOfConnectedRooms = 0;
	rm->numberOfNeighborRooms = 0;
	rm->numberOfDoorPositions = 0;
	rm->numberOfCombinedRooms = 0;
	rm->numberOfOccupiedSpawnPositions = 0;
	rm->flags = flags;
	rm->prev = NULL;
	rm->next = NULL;
	rm->neighborRoomIds = (int*)malloc((4) * sizeof(int));
	rm->connectedRoomIds = (int*)malloc((4) * sizeof(int));
	rm->combinedRoomIds = (int*)malloc((4) * sizeof(int));
	rm->spawnPositions = malloc(sizeof(int) * rm->size);
	rm->wasCombined = 0;

	//TODO: Use memcpy in the future
	rm->positionList = malloc(sizeof *rm->positionList * rm->size);
	if (rm->positionList)
	{
		for (i = 0; i < rm->size; i++)
		{
			rm->positionList[i] = malloc(sizeof(int) * 2);
		}
	}

	rm->doorPositions = malloc(sizeof *rm->doorPositions * rm->size);
	if (rm->doorPositions)
	{
		for (i = 0; i < rm->size; i++)
		{
			rm->doorPositions[i] = malloc(sizeof(int) * 2);
		}
	}

	i = 0;

	for (y = rm->y; y < rm->y + rm->height; y ++) {
		for (x = rm->x; x < rm->x + rm->width; x ++) {
			rm->positionList[i][0] = x;
			rm->positionList[i][1] = y;

			i ++;
		}
	}

	/*for (y = 0; y < WINDOW_HEIGHT; y ++) {
		addPosition = 0;

		for (x = 0; x < WINDOW_WIDTH; x ++) {
			if (roomMap[x][y] == id) {
				xAvg += x;
				xAvgLen ++;
				addPosition = 1;
				rm->positionList[positionIndex][0] = x;
				rm->positionList[positionIndex][1] = y;

				positionIndex ++;
			}
		}

		if (addPosition) {
			yAvg += y;
			yAvgLen ++;
		}
	}*/

	//rm->centerX = xAvg / xAvgLen;
	//rm->centerY = yAvg / yAvgLen;

	if (ROOMS == NULL) {
		ROOMS = rm;
	} else {
		ptr = ROOMS;

		while (ptr->next) {
			ptr = ptr->next;
		}

		ptr->next = rm;
		rm->prev = ptr;
	}

	return rm;
}

room *getRooms() {
	return ROOMS;
}

room *getRoomViaId(int id) {
	room *roomPtr = ROOMS;

	while (roomPtr) {
		if (roomPtr->id == id) {
			return roomPtr;
		}

		roomPtr = roomPtr->next;
	}

	printf("FATAL: Failed to return room of ID %i\n", id);

	return NULL;
}

room *getRoomWithFlags(unsigned int flags) {
	room *roomPtr = ROOMS;

	while (roomPtr) {
		if (roomPtr->flags & flags) {
			return roomPtr;
		}

		roomPtr = roomPtr->next;
	}

	printf("FATAL: Failed to return room with flags %i\n", flags);

	return NULL;
}

void deleteRoom(room *rm) {
	if (rm == ROOMS) {
		ROOMS = rm->next;
	} else {
		rm->prev->next = rm->next;

		if (rm->next) {
			rm->next->prev = rm->prev;
		}
	}

	free(rm->neighborRoomIds);
	free(rm);
}

void deleteAllRooms() {
	room *next, *ptr = ROOMS;

	printf("Deleting all rooms...\n");

	while (ptr != NULL) {
		next = ptr->next;

		deleteRoom(ptr);

		ptr = next;
	}

	ROOMS = NULL; //Just in case...?
}

void combineRoom(room *srcRoom, room *dstRoom) {
	int i, newSize, **newPositionList;

	srcRoom->wasCombined = 1;
	newSize = srcRoom->size + dstRoom->size;

	newPositionList = malloc(sizeof *newPositionList * newSize);

	for (i = 0; i < newSize; i++) {
		newPositionList[i] = malloc(sizeof(int) * 2);
	}

	for (i = 0; i < srcRoom->size; i++) {
		newPositionList[i][0] = srcRoom->positionList[i][0];
		newPositionList[i][1] = srcRoom->positionList[i][1];
	}

	for (i = 0; i < dstRoom->size; i++) {
		newPositionList[srcRoom->size + i][0] = dstRoom->positionList[i][0];
		newPositionList[srcRoom->size + i][1] = dstRoom->positionList[i][1];
	}

	for (i = 0; i < srcRoom->size; i++) {
		free(srcRoom->positionList[i]);
	}

	free(srcRoom->positionList);
	srcRoom->positionList = newPositionList;

	//deleteRoom(dstRoom);
}

void addRoomDoorPosition(room *srcRoom, int x, int y) {
	srcRoom->doorPositions[srcRoom->numberOfDoorPositions][0] = x;
	srcRoom->doorPositions[srcRoom->numberOfDoorPositions][1] = y;

	if (isPositionInRoom(srcRoom, x, y)) {
		claimSpawnPositionInRoom(srcRoom, x, y);
	}

	srcRoom->numberOfDoorPositions ++;
}

void addNeighbor(room *srcRoom, room *dstRoom) {
	if (!isNeighborWith(srcRoom, dstRoom)) {
		srcRoom->neighborRoomIds[srcRoom->numberOfNeighborRooms] = dstRoom->id;

		srcRoom->numberOfNeighborRooms++;
	}

	if (!isNeighborWith(dstRoom, srcRoom)) {
		dstRoom->neighborRoomIds[dstRoom->numberOfNeighborRooms] = srcRoom->id;

		dstRoom->numberOfNeighborRooms++;
	}
}

void connectRooms(room *srcRoom, room *dstRoom) {
	if (!isConnectedWith(srcRoom, dstRoom)) {
		srcRoom->connectedRoomIds[srcRoom->numberOfConnectedRooms] = dstRoom->id;

		srcRoom->numberOfConnectedRooms++;
	}

	if (!isConnectedWith(dstRoom, srcRoom)) {
		dstRoom->connectedRoomIds[dstRoom->numberOfConnectedRooms] = srcRoom->id;

		dstRoom->numberOfConnectedRooms++;
	}
}

int isPotentialCombinedRoom(room *srcRoom, room *dstRoom) {
	int i;

	if (srcRoom->id == dstRoom->id) {
		return 1;
	}

	for (i = 0; i < srcRoom->numberOfCombinedRooms; i++) {
		if (srcRoom->combinedRoomIds[i] == dstRoom->id) {
			return 1;
		}
	}

	return 0;
}

void addPotentialCombineRoom(room *srcRoom, room *dstRoom) {
	if (!isPotentialCombinedRoom(srcRoom, dstRoom)) {
		srcRoom->combinedRoomIds[srcRoom->numberOfCombinedRooms] = dstRoom->id;

		srcRoom->numberOfCombinedRooms++;
	}

	if (!isPotentialCombinedRoom(dstRoom, srcRoom)) {
		dstRoom->combinedRoomIds[dstRoom->numberOfCombinedRooms] = srcRoom->id;

		srcRoom->numberOfCombinedRooms++;
	}
}

int isPositionSpawnable(room *srcRoom, int x, int y) {
	int i, xx, yy;

	if (!isPositionInRoom(srcRoom, x, y)) {
		return 0;
	}

	for (i = 0; i < srcRoom->numberOfOccupiedSpawnPositions; i ++) {
		xx = srcRoom->positionList[srcRoom->spawnPositions[i]][0];
		yy = srcRoom->positionList[srcRoom->spawnPositions[i]][1];

		if (x == xx && y == yy) {
			return 0;
		}
	}

	return 1;
}

void getNewSpawnPosition(room *srcRoom, int coordArray[]) {
	int i, x, y, spawnIndex, invalid = 1;

	while (invalid) {
		spawnIndex = getRandomInt(0, clip(srcRoom->size - 1, 0, 9999));

		if (!srcRoom->size) {
			printf("*FATAL* Room is too small.\n");

			assert(!srcRoom->size);
		}

		x = srcRoom->positionList[spawnIndex][0];
		y = srcRoom->positionList[spawnIndex][1];
		invalid = 0;

		if (!isPositionWalkable(x, y)) {
			continue;
		}

		for (i = 0; i < srcRoom->numberOfOccupiedSpawnPositions; i ++) {
			if (srcRoom->spawnPositions[i] == spawnIndex) {
				x = -1;
				y = -1;
				invalid = 1;

				break;
			}
		}
	}

	if (x == -1 || y == -1) {
		printf("Can't find spawn position.\n");
	}

	assert(x > -1 && y > -1);

	srcRoom->spawnPositions[srcRoom->numberOfOccupiedSpawnPositions] = spawnIndex;
	srcRoom->numberOfOccupiedSpawnPositions ++;
	coordArray[0] = x;
	coordArray[1] = y;
}

void claimSpawnPositionInRoom(room *srcRoom, int x, int y) {
	int i, spawnIndex = -1, invalid = 0;

	for (i = 0; i < srcRoom->size; i ++) {
		if (srcRoom->positionList[i][0] == x && srcRoom->positionList[i][1] == y) {
			spawnIndex = i;

			break;
		}
	}

	if (spawnIndex == -1) {
		printf("*FATAL* Could not claim spawn position: Position not found.\n");

		assert(spawnIndex > -1);
	}

	for (i = 0; i < srcRoom->numberOfOccupiedSpawnPositions; i ++) {
		if (srcRoom->spawnPositions[i] == spawnIndex) {
			invalid = 1;

			break;
		}
	}

	if (invalid) {
		printf("*FATAL* Could not claim spawn position: Position already claimed.\n");

		assert(!invalid);
	}

	srcRoom->spawnPositions[srcRoom->numberOfOccupiedSpawnPositions] = spawnIndex;
	srcRoom->numberOfOccupiedSpawnPositions ++;
}

void placeItemInRoom(room *srcRoom, item *itm) {
	int pos[2];

	getNewSpawnPosition(srcRoom, pos);

	itm->x = pos[0];
	itm->y = pos[1];
}

void createAndPlaceItemInRoom(room *srcRoom, void (*createItem)(int, int)) {
	int pos[2];

	getNewSpawnPosition(srcRoom, pos);

	createItem(pos[0], pos[1]);
}

int isNeighborWith(room *srcRoom, room *dstRoom) {
	int i;

	if (srcRoom->id == dstRoom->id) {
		return 1;
	}

	for (i = 0; i < srcRoom->numberOfNeighborRooms; i++) {
		if (srcRoom->neighborRoomIds[i] == dstRoom->id) {
			return 1;
		}
	}

	return 0;
}

int isNeighborWithId(room *srcRoom, int dstRoomId) {
	int i;

	if (srcRoom->id == dstRoomId) {
		return 1;
	}

	for (i = 0; i < srcRoom->numberOfNeighborRooms; i++) {
		if (srcRoom->neighborRoomIds[i] == dstRoomId) {
			return 1;
		}
	}

	return 0;
}

int isConnectedWith(room *srcRoom, room *dstRoom) {
	int i;

	if (srcRoom->id == dstRoom->id) {
		return 1;
	}

	for (i = 0; i < srcRoom->numberOfConnectedRooms; i++) {
		if (srcRoom->connectedRoomIds[i] == dstRoom->id) {
			return 1;
		}
	}

	return 0;
}

int isPositionInRoom(room *roomPtr, int x, int y) {
	int positionIndex;

	for (positionIndex = 0; positionIndex < roomPtr->size; positionIndex ++) {
		if (x == roomPtr->positionList[positionIndex][0] && y == roomPtr->positionList[positionIndex][1]) {
			return 1;
		}
	}

	return 0;
}