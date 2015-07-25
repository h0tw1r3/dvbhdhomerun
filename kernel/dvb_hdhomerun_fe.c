/*
 * dvb_hdhomerun_fe.c, skeleton driver for the HDHomeRun devices
 *
 * Copyright (C) 2010 Villy Thomsen <tfylliv@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/version.h>

#include <linux/platform_device.h>

#include "dvb_demux.h"
#include "dvb_frontend.h"
#include "dvb_net.h"
#include "dvbdev.h"
#include "dmxdev.h"

#include "dvb_hdhomerun_debug.h"
#include "dvb_hdhomerun_core.h"
#include "dvb_hdhomerun_control_messages.h"

MODULE_AUTHOR("Villy Thomsen");
MODULE_DESCRIPTION("DVB frontend for HDHomeRun");
MODULE_LICENSE("GPL");
MODULE_VERSION(HDHOMERUN_VERSION);

struct dvb_hdhomerun_fe_state {
	struct dvb_frontend frontend;
	int id;
};

extern int hdhomerun_debug_mask;

static int dvb_hdhomerun_fe_read_status(struct dvb_frontend* fe, enum fe_status* status)
{
	struct dvbhdhomerun_control_mesg mesg;
	struct dvb_hdhomerun_fe_state* state = fe->demodulator_priv;

	DEBUG_FUNC(1);

	mesg.type = DVB_HDHOMERUN_FE_READ_STATUS;
	mesg.id = state->id;
	hdhomerun_control_post_and_wait(&mesg);

	*status = mesg.u.frontend_status;

	return 0;
}

static int dvb_hdhomerun_fe_read_ber(struct dvb_frontend* fe, u32* ber)
{
	DEBUG_FUNC(1);
	*ber = 0;

	return 0;
}

static int dvb_hdhomerun_fe_read_signal_strength(struct dvb_frontend* fe, u16* strength)
{
	struct dvbhdhomerun_control_mesg mesg;
	struct dvb_hdhomerun_fe_state* state = fe->demodulator_priv;

	DEBUG_FUNC(1);

	mesg.type = DVB_HDHOMERUN_FE_READ_SIGNAL_STRENGTH;
	mesg.id = state->id;
	hdhomerun_control_post_and_wait(&mesg);

	*strength = mesg.u.signal_strength;

	return 0;
}

static int dvb_hdhomerun_fe_read_snr(struct dvb_frontend* fe, u16* snr)
{
	DEBUG_FUNC(1);
	*snr = 0;
	return 0;
}

static int dvb_hdhomerun_fe_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	DEBUG_FUNC(1);
	*ucblocks = 0;
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
static int dvb_hdhomerun_fe_get_frontend(struct dvb_frontend* fe)
#else
static int dvb_hdhomerun_fe_get_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters *p)
#endif
{
	DEBUG_FUNC(1);
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
static int dvb_hdhomerun_fe_set_frontend(struct dvb_frontend* fe)
#else
static int dvb_hdhomerun_fe_set_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters *p)
#endif
{
	struct dvb_hdhomerun_fe_state* state = fe->demodulator_priv;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	struct dtv_frontend_properties *p = &fe->dtv_property_cache;
#endif

	DEBUG_FUNC(1);
	if (fe->ops.tuner_ops.set_params) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
		fe->ops.tuner_ops.set_params(fe);
#else
		fe->ops.tuner_ops.set_params(fe,p);
#endif
		if (fe->ops.i2c_gate_ctrl)
			fe->ops.i2c_gate_ctrl(fe, 0);
	}

	DEBUG_OUT(HDHOMERUN_FE, "FE_SET_FRONTEND, freq: %d\n",
             p->frequency);

	{
		struct dvbhdhomerun_control_mesg mesg;
		mesg.type = DVB_HDHOMERUN_FE_SET_FRONTEND;
		mesg.id = state->id;
		mesg.u.frequency = p->frequency;
		
		hdhomerun_control_post_and_wait(&mesg);
	}

	return 0;
}

static int dvb_hdhomerun_fe_sleep(struct dvb_frontend* fe)
{
	DEBUG_FUNC(1);
	return 0;
}

static int dvb_hdhomerun_fe_init(struct dvb_frontend* fe)
{
	DEBUG_FUNC(1);
	
	/* Don't really need to do anything here */

	return 0;
}

static void dvb_hdhomerun_fe_release(struct dvb_frontend* fe)
{
	struct dvb_hdhomerun_fe_state* state = fe->demodulator_priv;

	DEBUG_FUNC(1);

	kfree(state);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
static int dvb_hdhomerun_fe_get_frontend_algo(struct dvb_frontend *fe)
#else
static enum dvbfe_algo dvb_hdhomerun_fe_get_frontend_algo(struct dvb_frontend *fe)
#endif
{
	DEBUG_FUNC(1);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
	return 1; // The two drivers that use get_grontend_algo in debian lenny return 1 for HW.
#else
	return DVBFE_ALGO_HW; // This is actually 0. hmm.
#endif
}

#ifdef S2_LIPLIANIN
static int dvb_hdhomerun_fe_tune(struct dvb_frontend *fe, struct dvb_frontend_parameters *params)
{
	DEBUG_FUNC(1);

	return dvb_hdhomerun_fe_set_frontend(fe, params);
}
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
static int dvb_hdhomerun_fe_tune(struct dvb_frontend *fe, bool re_tune, 
#else
static int dvb_hdhomerun_fe_tune(struct dvb_frontend *fe, struct dvb_frontend_parameters *params,
#endif
					unsigned int mode_flags, unsigned int *delay, enum fe_status *status)
{
   int ret;
	DEBUG_FUNC(1);

	//*delay = HZ / 5;
	*delay = 60 * HZ;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	ret = dvb_hdhomerun_fe_set_frontend(fe);
	if (ret)
           return ret;
#else
	if (params) {
		ret = dvb_hdhomerun_fe_set_frontend(fe, params);
		if (ret)
                        return ret;
	}
#endif

	return dvb_hdhomerun_fe_read_status(fe, status);
}
#endif // S2_LIPLIANIN


/* Setup/Init functions */

/* DVB_T */
static struct dvb_frontend_ops dvb_hdhomerun_fe_ofdm_ops;

struct dvb_frontend *dvb_hdhomerun_fe_attach_dvbt(int id)
{
        struct dvb_hdhomerun_fe_state* state = NULL;

        DEBUG_FUNC(1);

        /* allocate memory for the internal state */
        state = kzalloc(sizeof(struct dvb_hdhomerun_fe_state), GFP_KERNEL);
        if (state == NULL) goto error;

        DEBUG_OUT(HDHOMERUN_FE, "Attaching a DVB-T HDHomeRun id %d\n", id);
        DEBUG_OUT(HDHOMERUN_FE, "Attaching id %d\n", id);
        state->id = id;

        /* create dvb_frontend */
        memcpy(&state->frontend.ops, &dvb_hdhomerun_fe_ofdm_ops, sizeof(struct dvb_frontend_ops));
        state->frontend.demodulator_priv = state;
        return &state->frontend;

error:
        kfree(state);
        return NULL;
}

static struct dvb_frontend_ops dvb_hdhomerun_fe_ofdm_ops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
        .delsys = { SYS_DVBT },
#endif
        .info = {
                .name                   = "HDHomeRun DVB-T",
                .type                   = FE_OFDM,
                .frequency_stepsize     = 62500,
                .frequency_min          = 50500000,
                .frequency_max          = 862000000,
        .caps =
		    FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
		    FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
		    FE_CAN_QPSK | FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
		    FE_CAN_TRANSMISSION_MODE_AUTO | FE_CAN_GUARD_INTERVAL_AUTO
        },

        .release = dvb_hdhomerun_fe_release,

        .init = dvb_hdhomerun_fe_init,
        .sleep = dvb_hdhomerun_fe_sleep,

        .set_frontend = dvb_hdhomerun_fe_set_frontend,
        .get_frontend = dvb_hdhomerun_fe_get_frontend,

        .read_status = dvb_hdhomerun_fe_read_status,
        .read_ber = dvb_hdhomerun_fe_read_ber,
        .read_signal_strength = dvb_hdhomerun_fe_read_signal_strength,
        .read_snr = dvb_hdhomerun_fe_read_snr,
        .read_ucblocks = dvb_hdhomerun_fe_read_ucblocks,

        .tune = dvb_hdhomerun_fe_tune,
        .get_frontend_algo = dvb_hdhomerun_fe_get_frontend_algo,
};

/* DVB_C */
static struct dvb_frontend_ops dvb_hdhomerun_fe_qam_ops;

struct dvb_frontend *dvb_hdhomerun_fe_attach_dvbc(int id)
{
	struct dvb_hdhomerun_fe_state* state = NULL;

	DEBUG_FUNC(1);

	/* allocate memory for the internal state */
	state = kzalloc(sizeof(struct dvb_hdhomerun_fe_state), GFP_KERNEL);
	if (state == NULL) goto error;

	DEBUG_OUT(HDHOMERUN_FE, "Attaching a DVB-C HDHomeRun id %d\n", id);	
	DEBUG_OUT(HDHOMERUN_FE, "Attaching id %d\n", id);
	state->id = id;

	/* create dvb_frontend */
	memcpy(&state->frontend.ops, &dvb_hdhomerun_fe_qam_ops, sizeof(struct dvb_frontend_ops));
	state->frontend.demodulator_priv = state;
	return &state->frontend;

error:
	kfree(state);
	return NULL;
}

static struct dvb_frontend_ops dvb_hdhomerun_fe_qam_ops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
   .delsys = { SYS_DVBC_ANNEX_A },
#endif
	.info = {
		.name			= "HDHomeRun DVB-C",
		.type			= FE_QAM,
		.frequency_stepsize	= 62500,
		.frequency_min		= 51000000,
		.frequency_max		= 858000000,
		.symbol_rate_min	= (57840000/2)/64,     /* SACLK/64 == (XIN/2)/64 */
		.symbol_rate_max	= (57840000/2)/4,      /* SACLK/4 */
		.caps = FE_CAN_QAM_16 | FE_CAN_QAM_32 | FE_CAN_QAM_64 |
			FE_CAN_QAM_128 | FE_CAN_QAM_256 | FE_CAN_QAM_AUTO |
			FE_CAN_FEC_AUTO | FE_CAN_INVERSION_AUTO
	},

	.release = dvb_hdhomerun_fe_release,

	.init = dvb_hdhomerun_fe_init,
	.sleep = dvb_hdhomerun_fe_sleep,

	.set_frontend = dvb_hdhomerun_fe_set_frontend,
	.get_frontend = dvb_hdhomerun_fe_get_frontend,

	.read_status = dvb_hdhomerun_fe_read_status,
	.read_ber = dvb_hdhomerun_fe_read_ber,
	.read_signal_strength = dvb_hdhomerun_fe_read_signal_strength,
	.read_snr = dvb_hdhomerun_fe_read_snr,
	.read_ucblocks = dvb_hdhomerun_fe_read_ucblocks,

	.tune = dvb_hdhomerun_fe_tune,
	.get_frontend_algo = dvb_hdhomerun_fe_get_frontend_algo,
};

/* ATSC */
static struct dvb_frontend_ops dvb_hdhomerun_fe_atsc_ops;

struct dvb_frontend *dvb_hdhomerun_fe_attach_atsc(int id)
{
	struct dvb_hdhomerun_fe_state* state = NULL;

	DEBUG_FUNC(1);

	/* allocate memory for the internal state */
	state = kzalloc(sizeof(struct dvb_hdhomerun_fe_state), GFP_KERNEL);
	if (state == NULL) goto error;

	DEBUG_OUT(HDHOMERUN_FE, "Attaching an ATSC HDHomeRun id %d\n", id);	
	DEBUG_OUT(HDHOMERUN_FE, "Attaching id %d\n", id);
	state->id = id;

	/* create dvb_frontend */
	memcpy(&state->frontend.ops, &dvb_hdhomerun_fe_atsc_ops, sizeof(struct dvb_frontend_ops));
	state->frontend.demodulator_priv = state;
	return &state->frontend;

error:
	kfree(state);
	return NULL;
}

static struct dvb_frontend_ops dvb_hdhomerun_fe_atsc_ops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
   .delsys = { SYS_ATSC },
#endif
	.info = {
		.name			= "HDHomeRun ATSC",
		.type			= FE_ATSC,
		.frequency_stepsize	= 62500,
		.frequency_min		= 54000000,
		.frequency_max		= 858000000,
		.symbol_rate_min	= (57840000/2)/64,     /* SACLK/64 == (XIN/2)/64 */
		.symbol_rate_max	= (57840000/2)/4,      /* SACLK/4 */
		.caps = FE_CAN_FEC_AUTO | FE_CAN_INVERSION_AUTO | 
              FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_QAM_128 |
              FE_CAN_QAM_256 | FE_CAN_QAM_AUTO |
		        FE_CAN_8VSB | FE_CAN_16VSB
	},

	.release = dvb_hdhomerun_fe_release,

	.init = dvb_hdhomerun_fe_init,
	.sleep = dvb_hdhomerun_fe_sleep,

	.set_frontend = dvb_hdhomerun_fe_set_frontend,
	.get_frontend = dvb_hdhomerun_fe_get_frontend,

	.read_status = dvb_hdhomerun_fe_read_status,
	.read_ber = dvb_hdhomerun_fe_read_ber,
	.read_signal_strength = dvb_hdhomerun_fe_read_signal_strength,
	.read_snr = dvb_hdhomerun_fe_read_snr,
	.read_ucblocks = dvb_hdhomerun_fe_read_ucblocks,

	.tune = dvb_hdhomerun_fe_tune,
	.get_frontend_algo = dvb_hdhomerun_fe_get_frontend_algo,
};


EXPORT_SYMBOL(dvb_hdhomerun_fe_attach_dvbc);
EXPORT_SYMBOL(dvb_hdhomerun_fe_attach_atsc);
EXPORT_SYMBOL(dvb_hdhomerun_fe_attach_dvbt);

