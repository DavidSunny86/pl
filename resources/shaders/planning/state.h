

// structs

struct state;

// functions

float getInvoEnergies      ();
vec4  getInvoGraftPositions(in uint index);
vec4  getInvoGraftNormals  (in uint index);
float getInvoGraftRadii    (in uint index);
uint  getInvoGraftCounts   ();

float getInvoEnergies      (in uint invoIndex);
vec4  getInvoGraftPositions(in uint invoIndex, in uint graftIndex);
vec4  getInvoGraftNormals  (in uint invoIndex, in uint graftIndex);
float getInvoGraftRadii    (in uint invoIndex, in uint graftIndex);
uint  getInvoGraftCounts   (in uint invoIndex);

float getGroupEnergies      ();
vec4  getGroupGraftPositions(in uint index);
vec4  getGroupGraftNormals  (in uint index);
float getGroupGraftRadii    (in uint index);
uint  getGroupGraftCounts   ();

void setInvoEnergies      (in float energy);
void setInvoGraftPositions(in uint index, in vec4 position);
void setInvoGraftNormals  (in uint index, in vec4 normal);
void setInvoGraftRadii    (in uint index, in float radius);
void setInvoGraftCounts   (in uint count);

void setGroupEnergies      (in float energy);
void setGroupGraftPositions(in uint index, in vec4 position);
void setGroupGraftNormals  (in uint index, in vec4 normal);
void setGroupGraftRadii    (in uint index, in float radius);
void setGroupGraftCounts   (in uint count);

void translateGraft(inout vec4 graftPosition, in vec4 graftNormal, in float graftRadius, inout uint seed);
void rotateGraft(inout vec4 graftNormal, inout uint seed);
void perturbState(inout state s, inout uint seed);
void addCap(inout state s, inout uint seed);
void removeCap(inout state s, inout uint seed);
void copyGroupState(inout state s);
void copyLocalState(inout state s);
void copyPreviousState(inout state s);
void progressState(inout state s, inout uint seed);
void storeState(in state s, inout uint seed);

void storeLocalToGroup(in uint localIndex);
void getShuffledIndexArray(inout uint array[PL_STAGE_0_WORKGROUP_SIZE], inout uint seed);
void updateGroupStates(inout uint seed);
