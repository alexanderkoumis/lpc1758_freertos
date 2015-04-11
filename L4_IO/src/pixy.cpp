#include "pixy.hpp"

#include "LPC17xx.h"
#include "ssp1.h"
#include "printf_lib.h"

namespace konix {

static PixyBlockType last_block_type;


static inline unsigned char konix_exchange_byte(LPC_SSP_TypeDef *pSSP, char out) {
	pSSP->DR = out;
	while(pSSP->SR & (1 << 4)); // Wait until SSP is busy
	return pSSP->DR;
}

Pixy::Pixy() :
    state_(START),
	recv_word_(0x00),
	img_count_(0),
	block_count_(0) {
  while(LPC_SSP1->SR & (1 << 4));
  LPC_GPIO0->FIOCLR = (1 << 16); // P0[16] as SSP1
}

void Pixy::StateMachine() {
  switch(state_) {
    case START: {
      while ((recv_word_ != 0xaa55)) {
//        recv_word_ = ReadWord();
//        u0_dbg_printf("recv_word_: %04x\n", recv_word_);
    	unsigned char recv_char = konix_exchange_byte(LPC_SSP1, 0x00);
    	char recv_char_2 = recv_char;
        u0_dbg_printf("%x, %u, %d, %x, %u, %d\n", recv_char, recv_char, recv_char,
        										 (~recv_char) + 0x01,
												 (~recv_char) + 0x01,
												 (~recv_char) + 0x01);
      }
      recv_word_ = ReadWord();
      while ((recv_word_ != 0xaa55) && (recv_word_ != 0xaa56)) {
        recv_word_ = ReadWord();
      }

      last_block_type = (recv_word_ == 0xaa56) ? NORMAL : COLOR;
      state_ = READING_SAME_FRAME;
      break;
    }
    case READING_FIRST_WORD: {
      recv_word_ = ReadWord();
      if (recv_word_ == 0xaa55) {
        state_ = (ReadWord() == 0xaa55) ? READING_NEW_FRAME : READING_SAME_FRAME;
      }
      else {
        u0_dbg_printf("Something's wrong. First word should have been 0xaa55.\n"
                      "It wasn't. It was %02x.\n", recv_word_);
      }
      break;
    }
    case READING_NEW_FRAME: {
      img_count_++;
      ReadWord();                             // Checksum
      state_ = READING_SAME_FRAME;
      break;
    }
    case READING_SAME_FRAME: {
      curr_block_.signature_ = ReadWord();
      curr_block_.x_ = ReadWord();
      curr_block_.y_ = ReadWord();
      curr_block_.width_ = ReadWord();
      curr_block_.height_ = ReadWord();
      curr_block_.angle_ = ReadWord();
      state_ = DONE;
      break;
    }
    case DONE: {
      u0_dbg_printf("Image #%d - Block #%d\n"
    		        "  Signature   : %02x\n"
					"  Coordinates : [%02x x %02x]\n"
					"  Size        : [%02x x %02x]\n"
					"  Angle       : %02x\n",
					img_count_, block_count_++,
					curr_block_.signature_,
					curr_block_.x_, curr_block_.y_,
					curr_block_.height_, curr_block_.width_,
					curr_block_.angle_);
      state_ = READING_FIRST_WORD;
      break;
    }
    default: {
      u0_dbg_printf("Error! State machine should not hit default case!\n");
      break;
    }
  }
}

void Pixy::PrintPixyStream() {
  u0_dbg_printf("LPC_SSP1->DR: %x\n", ssp1_exchange_byte(0x00));
}

__inline uint16_t Pixy::ReadWord() {
  uint8_t recv_byte = ssp1_exchange_byte(0x00);
  if (recv_byte == 0x55) {
	  u0_dbg_printf("POOPPY\n");
  }
  return ((ssp1_exchange_byte(0x00) << 8) | (0x00 | recv_byte));
}

} // namespace konix
