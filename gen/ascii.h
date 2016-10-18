/**
 * @file
 * @brief     ASCII characters.
 * @details   Definition of ASCII control codes.
 * @author    王文佑
 * @date      2015.10.06
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_ASCII_H_
#define _GEN_ASCII_H_

/**
 * ASCII control codes.
 */
enum
{
    ASCII_NUL = 0x00,  ///< Null.
    ASCII_SOH = 0x01,  ///< Start of heading.
    ASCII_STX = 0x02,  ///< Start of text.
    ASCII_ETX = 0x03,  ///< End of text.
    ASCII_EOT = 0x04,  ///< End of transmission.
    ASCII_ENQ = 0x05,  ///< Enquiry.
    ASCII_ACK = 0x06,  ///< Acknowledge.
    ASCII_BEL = 0x07,  ///< Bell (beep).
    ASCII_BS  = 0x08,  ///< Backspace.
    ASCII_HT  = 0x09,  ///< Horizontal tab.
    ASCII_LF  = 0x0A,  ///< Line feed.
    ASCII_VT  = 0x0B,  ///< Vertical tab.
    ASCII_FF  = 0x0C,  ///< Form feed.
    ASCII_CR  = 0x0D,  ///< Carriage return.
    ASCII_SO  = 0x0E,  ///< Shift out.
    ASCII_SI  = 0x0F,  ///< Shift in.
    ASCII_DLE = 0x10,  ///< Data link escape.
    ASCII_DC1 = 0x11,  ///< Device control 1 (XON).
    ASCII_DC2 = 0x12,  ///< Device control 2.
    ASCII_DC3 = 0x13,  ///< Device control 3 (XOFF).
    ASCII_DC4 = 0x14,  ///< Device control 4.
    ASCII_NAK = 0x15,  ///< Negative acknowledge.
    ASCII_SYN = 0x16,  ///< Synchronous idle.
    ASCII_ETB = 0x17,  ///< End of transmission block.
    ASCII_CAN = 0x18,  ///< Cancel.
    ASCII_EM  = 0x19,  ///< End of medium.
    ASCII_SUB = 0x1A,  ///< Substitute.
    ASCII_ESC = 0x1B,  ///< Escape.
    ASCII_FS  = 0x1C,  ///< File separator.
    ASCII_GS  = 0x1D,  ///< Group separator.
    ASCII_RS  = 0x1E,  ///< Record separator.
    ASCII_US  = 0x1F,  ///< Unit separator.
    ASCII_DEL = 0x7F,  ///< Delete.
};

#endif
