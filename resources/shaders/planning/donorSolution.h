

// functions

void setDonorSolutionPositions(in vec3 position, in uint graftIndex);
void setDonorSolutionNormals  (in vec3 normal, in uint graftIndex);
void setDonorSolutionXAxes    (in vec3 xAxis, in uint graftIndex);
void setDonorSolutionSiteIndex(in uint siteIndex, in uint graftIndex);

vec3 getDonorSolutionPositions(in uint graftIndex);
vec3 getDonorSolutionNormals  (in uint graftIndex);
