#version 430

/**************************************************************/
/**************************  GLOBAL  **************************/
/**************************************************************/


#define PL_PI                             3.1415927
#define PL_FLT_MAX                        9999999.0
#define PL_MAX_GRAFTS_PER_SOLUTION        16
#define PL_MAX_DONOR_SITES                5
#define PL_MAX_CAP_TRIANGLES              2048
#define PL_MAX_GRAFT_RADIUS               4.0
#define PL_MIN_GRAFT_RADIUS               3.0
#define PL_GRAFT_LENGTH                   15.0
#define PL_MAX_GRAFT_ROTATION             10.0
#define PL_NUM_COMPARISION_DIRECTIONS     36


/**************************************************************/
/*************************  STAGE 0  **************************/
/**************************************************************/


#define PL_STAGE_0_NUM_WORKGROUPS              16
#define PL_STAGE_0_WORKGROUP_SIZE              64
#define PL_STAGE_0_SITE_OVERLAP_THRESHOLD      0.8
#define PL_STAGE_0_GRAFT_INTERSECTION_PENALTY  3.0
#define PL_STAGE_0_GRAFT_COUNT_PENALTY         5
#define PL_STAGE_0_MIN_PERTURBATION_FACTOR     0.05


/**************************************************************/
/*************************  STAGE 1  **************************/
/**************************************************************/


#define PL_STAGE_1_WORKGROUP_SIZE             1024
#define PL_STAGE_1_DONOR_CAP_RADIUS           (PL_MAX_GRAFT_RADIUS / cos(PL_MAX_GRAFT_ROTATION*0.0174532))


/**************************************************************/
/*************************  STAGE 2  **************************/
/**************************************************************/


#define PL_STAGE_2_WORKGROUP_SIZE               1024
#define PL_STAGE_2_NUM_COMPARISON_SLICES        32
#define PL_STAGE_2_NUM_COMPARISON_RADII         4
#define PL_STAGE_2_SLICE_ANGLE_INCREMENT        ((2.0 * PL_PI) / float(PL_STAGE_2_NUM_COMPARISON_SLICES))
#define PL_STAGE_2_RADIUS_INCREMENT(radius)     ((radius / float(PL_STAGE_2_NUM_COMPARISON_RADII)) - 0.01)
#define PL_STAGE_2_ANGULAR_DISTANCE_THRESHOLD   1.04666 // (30 degrees)

/**************************************************************/
/*************************  STAGE 3  **************************/
/**************************************************************/


#define PL_STAGE_3_WORKGROUP_SIZE               64
