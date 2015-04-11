#ifndef PIXY_HPP
#define PIXY_HPP

#include "stdint.h"

namespace konix {

enum PixyState {
	START,
	READING_SAME_FRAME,
	READING_FIRST_WORD,
	READING_NEW_FRAME,
	DONE
};

enum PixyBlockType {
	NORMAL,
	COLOR
};

struct PixyBlock {
  PixyBlock() :
	  signature_(0x00),
	  x_(0x00),
	  y_(0x00),
	  width_(0x00),
	  height_(0x00),
	  angle_(0x00) {}
  uint16_t signature_;
  uint16_t x_;
  uint16_t y_;
  uint16_t width_;
  uint16_t height_ ;
  uint16_t angle_;
};

class Pixy {
 public:
  Pixy();
  void StateMachine();
  void PrintPixyStream();
 private:
  PixyBlock curr_block_;
  uint16_t ReadWord();
  PixyState state_;
  uint16_t recv_word_;
  int img_count_;
  int block_count_;

};


} // namespace konix

#endif
