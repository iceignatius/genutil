/**
 * @file
 * @brief     IP Constants.
 * @details   Definition of constants of IP parameters.
 * @author    王文佑
 * @date      2014.03.12
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NET_IPCONST_H_
#define _GEN_NET_IPCONST_H_

#include "iptype.h"

/**
 * @name IPv4
 * @{
 */
static const ipv4_t ipv4_const_invalid   = { {   0,   0,   0,   0 } };  ///< Invalid IP.
static const ipv4_t ipv4_const_any       = { {   0,   0,   0,   0 } };  ///< Unspecific IP.
static const ipv4_t ipv4_const_loop      = { { 127,   0,   0,   1 } };  ///< Loop back IP.
static const ipv4_t ipv4_const_broadcast = { { 255, 255, 255, 255 } };  ///< Broadcast IP.
/**
 * @}
 */

/**
 * @name IPv6
 * @{
 */
static const ipv6_t ipv6_const_invalid = { { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} } };  ///< Invalid IP.
/**
 * @}
 */

/**
 * @name MAC Address
 * @{
 */
static const macaddr_t macaddr_const_invalid = { { 0, 0, 0, 0, 0, 0 } };  ///< Invalid MAC address.
/**
 * @}
 */

#endif
