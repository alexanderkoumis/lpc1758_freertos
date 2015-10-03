#ifndef PIXY_CONFIG_HPP
#define PIXY_CONFIG_HPP

const uint32_t CHIPS_AT_A_TIME    = 200;
const uint32_t CHIPS_TO_CALIB     = 1000;
const uint32_t NUM_TIMES_FOR_CNT  = 2;
const float EMA_ALPHA_ADJ         = 0.01f;
const float CHIP_PROXIM_TOLERANCE = 0.5f;
const float CHIP_LOC_EMA_ALPHA    = 0.90f; // higher - new values weigh more
const float CHIP_COLOR_EMA_ALPHA  = 0.95f; // lower - old values weigh more
const enum SEEN_CHIP_ALGO {STUPID=0, DOWN_RIGHT=1} eSeenChipAlgo = STUPID;

#endif
