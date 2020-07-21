/**
 * @file ds.h
 * @brief Data sets
 * @note Copyright (C) 2011 Richard Cochran <richardcochran@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef HAVE_DS_H
#define HAVE_DS_H

#include "ddt.h"
#include "fault.h"
#include "filter.h"

/* clock data sets */

#define DDS_TWO_STEP_FLAG (1<<0)
#define DDS_SLAVE_ONLY    (1<<1)

struct defaultDS {
	UInteger8            flags;
	UInteger8            reserved1;
	UInteger16           numberPorts;
	UInteger8            priority1;
	struct ClockQuality  clockQuality;
	UInteger8            priority2;
	struct ClockIdentity clockIdentity;
	UInteger8            domainNumber;
	UInteger8            reserved2;
} PACKED;

#define OUI_LEN 3
struct clock_description 
{
    struct  static_ptp_text productDescription;
    struct  static_ptp_text revisionData;
    struct  static_ptp_text userDescription;
    Octet   manufacturerIdentity[OUI_LEN];
};

struct default_ds 
{
	struct defaultDS dds;
	int free_running;
	int freq_est_interval; /*log seconds*/

    bool        grand_master_capable;       ///< @brief If TRUE, we can become Grand Master CLock (802.1AS only)
    bool        tcCapable;                  ///< @brief If TRUE, we can become a Transparent Clock (802.1AS only)
//  enum ClockMode  PresetClockMode;
	int stats_interval; /*log seconds*/
	int kernel_leap;
	int sanity_freq_limit;
	int time_source;
	struct clock_description clock_desc;
	enum filter_type delay_filter;
	int delay_filter_length;
    int transparent_capable;
};

struct dataset {
	UInteger8            priority1;
	struct ClockIdentity identity;
	struct ClockQuality  quality;
	UInteger8            priority2;
	UInteger16           stepsRemoved;
	struct PortIdentity  sender;
	struct PortIdentity  receiver;
};

struct currentDS {
	UInteger16   stepsRemoved;
	TimeInterval offsetFromMaster;
	TimeInterval meanPathDelay;
} PACKED;

struct parentDS {
	struct PortIdentity  parentPortIdentity;
	UInteger8            parentStats;
	UInteger8            reserved;
	UInteger16           observedParentOffsetScaledLogVariance;
	Integer32            observedParentClockPhaseChangeRate;
	UInteger8            grandmasterPriority1;
	struct ClockQuality  grandmasterClockQuality;
	UInteger8            grandmasterPriority2;
	struct ClockIdentity grandmasterIdentity;
} PACKED;

struct parent_ds {
	struct parentDS pds;
	struct ClockIdentity *ptl;
	unsigned int path_length;
};

#define CURRENT_UTC_OFFSET  35 /* 1 Jul 2012 */
#define INTERNAL_OSCILLATOR 0xA0


enum ClockMode
{
    ClockMode_Unknown = 0,          ///< @brief Unknown/unsupported
    ClockMode_OC,                   ///< @brief Ordinary Clock
    ClockMode_BC,                   ///< @brief Boundary Clock (when multiple ports only)
    ClockMode_TC,                   ///< @brief Transparent Clock (when multiple ports only)
    ClockMode_GM,                   ///< @brief Grand Master Clock
};


struct timePropertiesDS 
{
    Integer16    currentUtcOffset;
    UInteger8    flags;
    Enumeration8 timeSource;
    enum ClockMode      ClockMode;          ///< @brief The current mode of operation
} PACKED;

struct portDS {
	struct PortIdentity portIdentity;
	Enumeration8        portState;
	Integer8            logMinDelayReqInterval;
	TimeInterval        peerMeanPathDelay;
	Integer8            logAnnounceInterval;
	UInteger8           announceReceiptTimeout;
	Integer8            logSyncInterval;
	Enumeration8        delayMechanism;
	Integer8            logMinPdelayReqInterval;
	UInteger8           versionNumber;
    int                 tcCapable;
} PACKED;

#define FRI_ASAP (-128)
struct port_defaults {
	Integer64 asymmetry;
	Integer8 logAnnounceInterval;
	Integer8 logSyncInterval;
	Integer8 logMinDelayReqInterval;
	Integer8 logMinPdelayReqInterval;
	UInteger8 announceReceiptTimeout;
	UInteger8 syncReceiptTimeout;
	UInteger8 transportSpecific;
	int announce_span;
	int path_trace_enabled;
	int follow_up_info;
	int freq_est_interval; /*log seconds*/
	struct fault_interval flt_interval_pertype[FT_CNT];
	UInteger32 neighborPropDelayThresh; /*nanoseconds*/
};

#endif
