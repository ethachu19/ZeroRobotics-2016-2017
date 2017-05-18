float itemDockOffsets[3];

float myZRState[12];
float eState[12];

float dropZone[4];
float otherZone[4];

float startZRState[12];
float SPS2Loc[3];

float finalSPS[3];

int heldItem;
int SPSNum;
int actualItem;

#define triangleArea 0.33f

int currentState;

void SFMove(float target[3]) {
    api.setPositionTarget(target);
    /*
    if (distance(myZRState,target)<0.2f) {
        float accel[3];
        for (int i = 0; i < 3; i++) {
            accel[i] = (3.0f * target[i] - myZRState[i]) / (2.0f);
        }
        api.setPositionTarget(accel);
    }*/
}

static float distance(float a[3], float b[3]) {
    float disVec[3];
    mathVecSubtract(disVec, a, b, 3);
    return mathVecMagnitude(disVec, 3);
}

static float angle(float* a, float* b) {
    return mathVecInner(a, b, 3) / (mathVecMagnitude(a, 3) * mathVecMagnitude(b, 3));
}

bool isDockable(int itemID) {
    float itemState[12];
    float dirVec[3];
    game.getItemZRState(itemState, itemID);

    float d = distance(itemState, myZRState);
    mathVecSubtract(dirVec, itemState, myZRState, 3);

    return angle(&myZRState[6], dirVec) > 0.968f &&
            mathVecMagnitude( & myZRState[3], 3) < .01f &&
            game.isFacingCorrectItemSide(itemID) &&
            //game.hasItem(itemID) == 0 &&
            d < itemDockOffsets[itemID / 2] &&
            d > itemDockOffsets[itemID / 2] - 0.022f;

}

void getItemMoveLoc(float* loc, int itemID) {
    game.getItemZRState(loc, itemID);

    for (int i = 0; i < 3; i++) {
        loc[6 + i] *= (itemDockOffsets[itemID/2] - 0.018f);
        loc[i] += loc[6 + i];
    }
}

// static float NN(const float w[][2], float * input, int input_length) {
//     float acc = 0;
//     for (int i = 0; i < input_length; i++) {
//         acc += w[i][0] * input[i] * input[i] + w[i][1] * input[i];
//     }
//     return acc;
// }


static float time_est(float* ZRState,  float* target, bool n){
    float data[3];
    float dir[3];
    mathVecSubtract(dir, ZRState, target, 3);
    data[0] = mathVecNormalize(dir, 3);
    data[1] = (!n)?mathVecInner(&ZRState[3], dir, 3) : 0;
    data[2] = (!n)?mathVecMagnitude(&ZRState[3], 3):0;
    
    float w[3][2] = {
        {-20.320173263549805f, 43.533782958984375f},
        {-68.77410888671875f, -67.85869598388672f},
        {-56.58150863647461f, 8.543967247009277f},
    };
    
    float acc = 0;
    for (int i = 0; i < 3; i++) {
        acc += (w[i][0]  * data[i] + w[i][1]) * data[i];
    }
    return acc + 3.57881918549f;
}

/*
static float f_est(float input[4]) {
    const float w[4][3] = {
        {-0.1056557446718216f, 9.285199165344238f, 0.6537243723869324f},
        {-70.80497741699219f, -4.74289083480835f, -0.1338890939950943f},
        {-40.56275939941406f, 11.873205184936523f, -0.11371234059333801f},
        {-0.5532723665237427f, 1.0010371208190918f, -0.8263781666755676f},
    };
    return NN(w, input, 4);
}*/
/*
int getBestItem() {
    float otherZoneDist;
    float itemState[12];
    float score;

    best = -1000;
    int max_id = 0;
    float time_remaining = 180 - game.getCurrentTime();
    //float myfuel = f_est(data);
    
    int enemy_item_held = -1;
    for (int i = 0; i < 6; i++) {
        if(game.hasItem(i) == 2){
            enemy_item_held = i;
        }
    }

    for (int i = 0; i < 6; i++) {
        int otherNoisyTarget = -1;
        float dirVec[3];
        float itemLoc[3];
    
        //Otherwise iterate over each item
        for(int j = 0; j < 6; j++) {
            //1. Fetch the item loction
            game.getItemLoc(itemLoc, j);
            //2. Get direction vector
            mathVecSubtract(dirVec, itemLoc, eState, 3);
            //3. Compare that to their velocity vector(y)
            if(angle(dirVec, &eState[3]) > 0.9f) {//check if angle is > 10deg (cos(10deg) = 0.984807753)
                otherNoisyTarget = j;
                break;
            }
        }
        
        getItemMoveLoc(itemState, i);
        float score_per_second = (4 - (i / 2));
        
        if(SPSNum && i/2 == 1){
            score_per_second = 6;
        }
        
        otherZoneDist = distance(otherZone, itemState);

        float time_to = time_est(myZRState, itemState, false);
        float time_back = SPSNum?20:time_est(itemState, dropZone, true);
        float other_time_to = time_est(eState, itemState, false);
        
        if(enemy_item_held != -1){
            other_time_to = SPSNum?40:(time_est(itemState, otherZone, true) + time_est(eState, otherZone, false));
        }
        
        float other_time_taken = other_time_to + SPSNum?20:time_est(itemState, otherZone, true);
    

        if ((enemy_item_held != i || SPSNum?20:time_est(eState, otherZone, true) < 5.0f) && !game.itemInZone(i) && mathVecMagnitude(&itemState[3], 3) < 0.005f && (i != otherNoisyTarget || other_time_to + 2 > time_to || distance(myZRState, itemState) < 0.3f) && (SPSNum==0 || SPS2(i))){
            float enemy_time_total = time_remaining - other_time_taken;
            
            score = (enemy_time_total > 0 && (i==otherNoisyTarget || enemy_item_held == -1) && !SPSNum) ? enemy_time_total * score_per_second: 0; //TUNE!!!
            DEBUG(("ID %i es %f", i, score));

            if (otherZoneDist < 0.2f && game.hasItemBeenPickedUp(i)) {
                score += (time_remaining - time_to) * score_per_second;
                DEBUG(("cs %d, %f, %f", i, score, time_to));
            }
            
            float time_total = time_remaining - time_back - time_to;

            if(time_to/4 > game.getFuelRemaining()){
                score += -1000.0f;
                //DEBUG(("%f > %f or %f < 10", time_to/4, game.getFuelRemaining(), (time_remaining - time_back - time_to)));
            }
            if(time_total > 0){
                score += time_total * score_per_second;
            } 
            if(i == targetItem && time_to < 5.0f){
                best = 1000.0f;
                return i;
            }
            DEBUG(("ID %i s %f", i, score));
        } else {
            score = -1000.0f;
        }
        //DEBUG(("ItemID %i potential score %f", i, score));
        if (score > best) {
            best = score;
            max_id = i;
            DEBUG(("Best %i, s %f", i, best));
        }
    }

    float dirVec[3];
    mathVecSubtract(dirVec, dropZone, eState, 3);
    if (angle(dirVec, &eState[3]) > 0.9f) {
        createInput(myZRState, data, dropZone, true);
        float mytime = t_est(data);
        createInput(eState, data, dropZone, true);
        float theirtime = t_est(data);
        float points = (time_remaining - theirtime) * 8;

        if(points > max_score && mytime + 2.0f < theirtime){
            //max_score = -1000.0f; //force zone block lol
        }
    }

    targetItem = max_id;
    return max_id;
}*/

bool itemPickup(int itemID) {
    float movePos[12];
    float itemState[12];
    float toVec[3];

    game.getItemZRState(itemState, itemID);
    getItemMoveLoc(movePos, itemID);

    mathVecSubtract(toVec, movePos, myZRState, 3);
    mathVecNormalize(toVec, 3);

    //DEBUG(("AngleTo: %f", mathVecInner(toVec, & itemState[6], 3)));
    if (mathVecInner(toVec, &itemState[6], 3) > -0.184f) {
        DEBUG(("Going Around"));
        mathVecCross(&itemState[9],toVec,&itemState[6]);
        //DEBUG(("MovePos0: %f,%f,%f", inter[0],inter[1],inter[2]));
        mathVecCross(movePos,&itemState[9],toVec);
        mathVecNormalize(movePos,3);
        for (int i = 0; i < 3; i++) {
            movePos[i] = (itemDockOffsets[itemID / 2] + 0.036f) * (movePos[i] + toVec[i]*0.08f) + itemState[i];
        }
    }

    for (int i = 0; i < 3; i++) {
        otherZone[i] = -dropZone[i];
        itemState[6 + i] *= -1;
        finalSPS[i] = SPS2Loc[i];
    }
    SFMove(movePos);
    api.setAttitudeTarget(&itemState[6]);
    
    float d = distance(itemState, myZRState);
    mathVecSubtract(toVec, itemState, myZRState, 3);

    bool dockable = angle(&myZRState[6], toVec) > 0.968f &&
            mathVecMagnitude(&myZRState[3], 3) < .008f &&
            game.isFacingCorrectItemSide(itemID) &&
            //game.hasItem(itemID) == 0 &&
            d < itemDockOffsets[itemID / 2] &&
            d > itemDockOffsets[itemID / 2] - 0.022f;

    if (dockable && game.dockItem(itemID)) {
        heldItem = itemID;
        return true;
    }
    return false;
}

bool SPS2(int itemID) {
    float itemLoc[12];
    getItemMoveLoc(itemLoc, itemID);
    float toItemVec[3];

    mathVecSubtract(toItemVec, itemLoc, startZRState, 3);

    mathVecCross(SPS2Loc,toItemVec, itemLoc);
    mathVecCross(SPS2Loc, SPS2Loc, itemLoc);
    mathVecNormalize(SPS2Loc,3);

    float scalar = -triangleArea / mathVecMagnitude(toItemVec, 3);
    for (int i = 0; i < 3; i++) {
        SPS2Loc[i] = scalar*SPS2Loc[i] + (startZRState[i] + itemLoc[i]) / 2.0f;
    }

    //return (mathVecMagnitude(toItemVec, 3) > 0.1f);
    //return verifyBounds(SPS2Loc);
    return 0.6f > fabs(SPS2Loc[0]) && 0.75f > fabs(SPS2Loc[1]) && 0.6f > fabs(SPS2Loc[2]);
}
/*
static bool verifyBounds(float test[3]) {
    return 0.64f > fabsf(test[0]) && 0.80f > fabsf(test[1]) && 0.64f > fabsf(test[2]);
}*/

/*
bool placeSPS() {
    int itemNum = getBestItem();
    DEBUG(("BEST SPS: %d", itemNum));
    SPS2(itemNum);
    switch (SPSNum) {
    case 2:
        if (itemPickup(itemNum)) {
            game.dropSPS();
        }
        break;
    case 1:
        //DEBUG(("Distance %f", distance(finalSPS, myZRState)));
        if (distance(finalSPS, myZRState) < 0.08f) {
            game.dropSPS();
            game.getZone(dropZone);
            return true;
        } else {
            api.setPositionTarget(finalSPS);
        }
        break;
    }
    return false;
}*/

void init() {
    itemDockOffsets[0] = 0.173f;
    itemDockOffsets[1] = 0.160f;
    itemDockOffsets[2] = 0.146f;
    game.dropSPS();
    currentState = 0;
    api.getMyZRState(startZRState);
}

void loop() {
    api.setPosGains(0.42f, 0.74f, 3.44f);
    SPSNum = game.getNumSPSHeld();
    api.getMyZRState(myZRState);
    api.getOtherZRState(eState);
    
    float otherZoneDist;
    float itemState[12];
    float toVec[3];
    float score;

    float best = -1000;
    int max_id = 0;
    float time_remaining = 180 - game.getCurrentTime();
        
    float dirVec[3];
    float itemLoc[3];
    
    int enemy_item_held = -1;
    int otherNoisyTarget = -1;

    for (int i = 5; i > -1; i--) {
        if(game.hasItem(i) == 2){
            enemy_item_held = i;
        }
        
        getItemMoveLoc(itemLoc, i);
        mathVecSubtract(dirVec, itemLoc, eState, 3);
        if(angle(dirVec, &eState[3]) > 0.9f) {
            otherNoisyTarget = i;
        }
    }


    for (int i = 0; i < 6; i++) {

        getItemMoveLoc(itemState, i);

        mathVecSubtract(toVec, itemState, myZRState, 3);
        
        float score_per_second = (4 - (i / 2)) + (SPSNum && i/2 == 1);
        
        score_per_second += (i/2 == 1)*2;
        
        otherZoneDist = distance(otherZone, itemState);

        float time_to = time_est(myZRState, itemState, false);
        float time_back = SPSNum?20:time_est(itemState, dropZone, true);
        float other_time_to = time_est(eState, itemState, false);
        
        if(enemy_item_held != -1){
            other_time_to = SPSNum?40:(time_est(itemState, otherZone, true) + time_est(eState, otherZone, false));
        }
        
        bool inBounds = SPS2(i);
        
        if(SPSNum){
            time_to += time_est(itemState, SPS2Loc, true);
        }
        
        float other_time_taken = other_time_to + SPSNum?20:time_est(itemState, otherZone, true);
    
        if ((enemy_item_held != i || SPSNum?20:time_est(eState, otherZone, false) < 5.0f) && !game.itemInZone(i) && mathVecMagnitude(&itemState[3], 3) < 0.005f && (i != otherNoisyTarget || other_time_to + 2 > time_to || distance(myZRState, itemState) < 0.3f) && (SPSNum==0 || inBounds)){
            float enemy_time_total = time_remaining - other_time_taken;
            
            score = (enemy_time_total > 0 && (i==otherNoisyTarget || enemy_item_held == -1) && !SPSNum) ? enemy_time_total * score_per_second: 0; //TUNE!!!

            if (angle(toVec, &itemState[6]) > -0.184f){
                time_to += 10.0f;
            }
            
            if (otherZoneDist < 0.2f && game.hasItemBeenPickedUp(i) && !(distance(eState, otherZone) < 0.15f && mathVecMagnitude(&eState[3], 3) < 0.01f)) {
                score += (time_remaining - time_to) * score_per_second;
            }
            
            float time_total = time_remaining - time_back - time_to;

            if(time_to > game.getFuelRemaining()*4){
                score -= 1000.0f;
                //DEBUG(("%f > %f or %f < 10", time_to/4, game.getFuelRemaining(), (time_remaining - time_back - time_to)));
            }
            if(time_total > 0){
                score += time_total * score_per_second;
            }
            if(time_to/4 > game.getFuelRemaining() || time_total<5){
                score += -1000.0f;
                //DEBUG(("%f > %f or %f < 10", time_to/4, game.getFuelRemaining(), (time_remaining - time_back - time_to)));
            }
            if(i == actualItem && time_to < 5.0f){
                best = 1000.0f;
                max_id =  i;
                break;
            }
        } else {
            score = -1000.0f;
        }
        //DEBUG(("ItemID %i potential score %f", i, score));
        if (score > best) {
            best = score;
            max_id = i;
        }
    }
    
    if(game.getCurrentTime()%2 == 0){
        actualItem = max_id;
    }
    

    switch (currentState) {
    case 0:
    {
        SPS2(actualItem);
        switch (SPSNum) {
            case 2:
                if (itemPickup(actualItem)) {
                    game.dropSPS();
                }
                break;
            case 1:
                //DEBUG(("Distance %f", distance(finalSPS, myZRState)));
                if (distance(finalSPS, myZRState) < 0.05f) {
                    game.dropSPS();
                    game.getZone(dropZone);
                    currentState = 2;
                } else {
                    SFMove(finalSPS);
                }
                break;
        }
        break;
    }
    case 1:
        if (itemPickup(actualItem))
            currentState = 2;
        if(best < 2){
            currentState = 3;
        }
        break;
    case 2:
    {
        float itemState[12];
        game.getItemZRState(itemState, heldItem);
        //float d1 = distance(itemState, dropZone);
        //mathVecAdd(itemState, itemState, &itemState[3], 3);
        //float d2 = distance(itemState, dropZone);
        if (distance(itemState, dropZone) < 0.038f){
            game.dropItem();
            currentState = 1;
        } else {
            float offset[3];
            //float fuzz = itemDockOffsets[heldItem/2] - 0.022f;
            //Determine the direction vector between us and the target
            mathVecSubtract(offset, dropZone, myZRState, 3);
            //Make it a unit vector for simplicity
            mathVecNormalize(offset, 3);
            //Rotate toward it
            api.setAttitudeTarget(offset);
    
            //Multiply the unit vector by the fuzz
            for (int i = 0; i < 3; i++) {
                offset[i] *= itemDockOffsets[heldItem/2] - 0.022f;
            }
            /*
            offset[0] *= fuzz;
            offset[1] *= fuzz;
            offset[2] *= fuzz;*/
    
            //Determine the position to move to
            mathVecSubtract(offset, dropZone, offset, 3);
    
            //Move there
            SFMove(offset);
        }            
        break;
    }
    case 3:
        api.setPositionTarget(dropZone);
    }
}
