//Makes medium triangle on starting side
//Takes larger items first
//takes items based on average of distance between zone and distance to sphere
//takes yellow items from enemy if he has them

//2D BEST: 54.71 against Item bot
//3D BEST: 41.90 against Item bot

#define HIGH_SPEED 1.45f
#define LOW_SPEED 0.025f
#define VERY_LOW_SPEED 0.01f

float Target[3];
float zoneInfo[4];

float itemDists[3];

char faze;

float SPSPositions[3][3];

float itemLocs[6][3];

float enemyZoneLoc[3];
int myItem;

ZRState myState;
ZRState otherState;

void init(){
    faze = 0;
	for(int i = 0;i < 2;i++){
	    SPSPositions[i][2] = 0.0;
	}
	//SPSPositions[2][2] = 0.23;

	itemDists[0] = 0.170;
	itemDists[1] = 0.155;
	itemDists[2] = 0.140;
	
	SPSPositions[0][0] = 0.0;
	SPSPositions[1][0] = -0.5;
	//SPSPositions[2][0] = 0.5;
	SPSPositions[0][1] = 0.15;
	SPSPositions[1][1] = 0.5;
	//SPSPositions[2][1] = 0.5;
	
	myItem = -1;
}

void loop(){
    if(api.getTime() == 1){
        if(myState[1] < 0){
            SPSPositions[1][0] *= -1;
            SPSPositions[1][1] *= -1;
            SPSPositions[0][1] *= -1;
            //SPSPositions[2][2] *= -1;
	    }
    }
    getItemLocs();
    
    api.getMyZRState(myState);
    api.getOtherZRState(otherState);

    //START check if they have yellow
    float distToItem0 = distToItem(0);
    float distToItem1 = distToItem(1);
    bool isInEnemy0 = isInEnemyZone(0);
    bool isInEnemy1 = isInEnemyZone(1);
    if(faze == 4 && (!isInEnemy0 && !isInEnemy1)){
        faze = 1;
    }else if(faze == 1){
        //DEBUG(("%d, %d, %f, %f, %d, %d",isInEnemy0,isInEnemy1,distToItem0,
        //distToItem1,isInMyZone(0),isInMyZone(1)));
        if(isInEnemy0 && (distToItem0 < 1.33*distToItem1 ||
        game.itemInZone(1)) && distBetween(otherState,itemLocs[0]) > 0.27){
            faze = 4;
        }else if(isInEnemy1 && (distToItem1 < 1.33*distToItem0 ||
        game.itemInZone(0)) && distBetween(otherState,itemLocs[1]) > 0.27){
            faze = 4;
        }
    }
    //END check if they have yellow
    
    switch(faze){
        case 0:     //drop SPS
            dropSPS();
            break;
        case 1:     //pick up items
            goToNearestItem();
            break;
        case 2:     //have item, put in zone
            putItemInZone();
            break;
        case 4:     //steal yellow things
            if(isInEnemy0){
                stealBigYellowThings(0);
            }else{
                stealBigYellowThings(1); 
            }
            break;
        default:
            break;
    }
    if(game.getFuelRemaining() > 3 || faze == 2){
        setPosAndGo((game.getFuelRemaining()/100)*1.5);
    }else{
        api.setPositionTarget(myState);
    }
    //DEBUG(("Faze: %d",faze));
    //DEBUG(("Target: %f,%f,%f",Target[0],Target[1],Target[2]));
    //if(game.getFuelRemaining() < 3 && faze == 1){
    //   memcpy(Target,enemyZoneLoc,3*sizeof(float));
    //   setPosAndGo(LOW_SPEED);
    //}
}

void getItemLocs(){
    for(int i = 0;i < 6;i++){
        game.getItemLoc(itemLocs[i],i);
    }
}
