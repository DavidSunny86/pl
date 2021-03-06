
layout (local_size_x = PL_STAGE_0_WORKGROUP_SIZE) in;

// shader storage buffer objects
layout (std430,  binding = 0)  buffer defectSiteDataBlock  { vec4 defectSiteData[]; };

// temporary buffer
layout (std430,  binding = 1)  buffer triangleAreaBlock    { float triangleAreaOverlap[]; };

// invocation output SSBOs
layout (std430,  binding = 2)  buffer invoEnergiesBlock    { float invoEnergies      [];  };
layout (std430,  binding = 3)  buffer invoPositionBlock    { vec4  invoGraftPositions[];  };
layout (std430,  binding = 4)  buffer invoNormalBlock      { vec4  invoGraftNormals  [];  };
layout (std430,  binding = 5)  buffer invoRadiiBlock       { float invoGraftRadii    [];  };
layout (std430,  binding = 6)  buffer invoGraftCountBlock  { uint  invoGraftCounts   [];  };

layout (std430,  binding = 7)  buffer groupEnergiesBlock   { float groupEnergies      []; };
layout (std430,  binding = 8)  buffer groupPositionBlock   { vec4  groupGraftPositions[]; };
layout (std430,  binding = 9)  buffer groupNormalBlock     { vec4  groupGraftNormals  []; };
layout (std430,  binding = 10) buffer groupRadiiBlock      { float groupGraftRadii    []; };
layout (std430,  binding = 11) buffer groupGraftCountBlock { uint  groupGraftCounts   []; };

uniform float uTemperature;
uniform uint  uLoadLocal;
