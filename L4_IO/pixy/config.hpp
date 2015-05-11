#ifndef PIXY_CONFIG_HPP
#define PIXY_CONFIG_HPP

const uint32_t CHIPS_AT_A_TIME    = 42;
const uint32_t CHIPS_TO_CALIB     = 1000;
const float CHIP_PROXIM_TOLERANCE = 0.07f;
const float CHIP_LOC_EMA_ALPHA    = 0.9f; // higher - new values weigh more
const float CHIP_COLOR_EMA_ALPHA  = 0.75f; // lower - old values weigh more

#endif
