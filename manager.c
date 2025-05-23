/* Required outside the BUILD_MANAGER #ifdef! */
#include "ast_config.h"

#ifdef BUILD_MANAGER /* no manager, no copyright */
/*
   Copyright (C) 2009 - 2010

   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org

   Dmitry Vagin <dmitry2004@yandex.ru>

   bg <bg_one@mail.ru>
*/

#include <asterisk/stringfields.h>		/* AST_DECLARE_STRING_FIELDS for asterisk/manager.h */
#include <asterisk/manager.h>			/* struct mansession, struct message ... */
#include <asterisk/strings.h>			/* ast_strlen_zero() */
#include <asterisk/callerid.h>			/* ast_describe_caller_presentation */

#include "ast_compat.h"				/* asterisk compatibility fixes */

#include "manager.h"
#include "chan_quectel.h"			/* devices */
#include "helpers.h"				/* ITEMS_OF() send_ccwa_set() send_reset() send_sms() send_ussd() */
#include "error.h"

static char * espace_newlines(const char * text);

static int manager_show_devices (struct mansession* s, const struct message* m)
{
	const char * id = astman_get_header (m, "ActionID");
	const char * device = astman_get_header (m, "Device");
	struct pvt * pvt;
	size_t count = 0;
	char buf[40];

	astman_send_listack (s, m, "Device status list will follow", "start");

	AST_RWLIST_RDLOCK (&gpublic->devices);
	AST_RWLIST_TRAVERSE (&gpublic->devices, pvt, entry)
	{
		ast_mutex_lock (&pvt->lock);
		if(ast_strlen_zero(device) || strcmp(device, PVT_ID(pvt)) == 0)
		{
			astman_append (s, "Event: QuectelDeviceEntry\r\n");
			if(!ast_strlen_zero (id))
				astman_append (s, "ActionID: %s\r\n", id);
			astman_append (s, "Device: %s\r\n", PVT_ID(pvt));
/* settings */          if (strcmp(CONF_UNIQ(pvt, quec_uac),"1") == 0) astman_append (s, "AudioSetting: %s\r\n", CONF_UNIQ(pvt, alsadev));
			else astman_append (s, "AudioSetting: %s\r\n", CONF_UNIQ(pvt, audio_tty));
			astman_append (s, "DataSetting: %s\r\n", CONF_UNIQ(pvt, data_tty));
			astman_append (s, "IMEISetting: %s\r\n", CONF_UNIQ(pvt, imei));
			astman_append (s, "IMSISetting: %s\r\n", CONF_UNIQ(pvt, imsi));
			astman_append (s, "ChannelLanguage: %s\r\n", CONF_SHARED(pvt, language));
			astman_append (s, "Context: %s\r\n", CONF_SHARED(pvt, context));
			astman_append (s, "Exten: %s\r\n", CONF_SHARED(pvt, exten));
			astman_append (s, "Group: %d\r\n", CONF_SHARED(pvt, group));
			astman_append (s, "RXGain: %d\r\n", CONF_SHARED(pvt, rxgain));
			astman_append (s, "TXGain: %d\r\n", CONF_SHARED(pvt, txgain));
			astman_append (s, "U2DIAG: %d\r\n", CONF_SHARED(pvt, u2diag));
			astman_append (s, "UseCallingPres: %s\r\n", CONF_SHARED(pvt, usecallingpres) ? "Yes" : "No");
			astman_append (s, "DefaultCallingPres: %s\r\n", CONF_SHARED(pvt, callingpres) < 0 ? "<Not set>" : ast_describe_caller_presentation (CONF_SHARED(pvt, callingpres)));
			astman_append (s, "AutoDeleteSMS: %s\r\n", CONF_SHARED(pvt, autodeletesms) ? "Yes" : "No");
			astman_append (s, "DisableSMS: %s\r\n", CONF_SHARED(pvt, disablesms) ? "Yes" : "No");
			astman_append (s, "ResetQuectel: %s\r\n", CONF_SHARED(pvt, resetquectel) ? "Yes" : "No");
			astman_append (s, "CallWaitingSetting: %s\r\n", dc_cw_setting2str(CONF_SHARED(pvt, callwaiting)));
			astman_append (s, "DTMF: %s\r\n", dc_dtmf_setting2str(CONF_SHARED(pvt, dtmf)));
			astman_append (s, "MinimalDTMFGap: %d\r\n", CONF_SHARED(pvt, mindtmfgap));
			astman_append (s, "MinimalDTMFDuration: %d\r\n", CONF_SHARED(pvt, mindtmfduration));
			astman_append (s, "MinimalDTMFInterval: %d\r\n", CONF_SHARED(pvt, mindtmfinterval));
/* state */
			astman_append (s, "State: %s\r\n", pvt_str_state(pvt));
                        if (strcmp(CONF_UNIQ(pvt, quec_uac),"1") == 0) astman_append (s, "AudioState: %s\r\n", CONF_UNIQ(pvt, alsadev));
			else astman_append (s, "AudioState: %s\r\n", PVT_STATE(pvt, audio_tty));
			astman_append (s, "DataState: %s\r\n", PVT_STATE(pvt, data_tty));
			astman_append (s, "Voice: %s\r\n", (pvt->has_voice) ? "Yes" : "No");
			astman_append (s, "SMS: %s\r\n", (pvt->has_sms) ? "Yes" : "No");
			astman_append (s, "Manufacturer: %s\r\n", pvt->manufacturer);
			astman_append (s, "Model: %s\r\n", pvt->model);
			astman_append (s, "Firmware: %s\r\n", pvt->firmware);
			astman_append (s, "IMEIState: %s\r\n", pvt->imei);
			astman_append (s, "IMSIState: %s\r\n", pvt->imsi);
			astman_append (s, "GSMRegistrationStatus: %s\r\n", GSM_regstate2str(pvt->gsm_reg_status));
			astman_append (s, "LTERegistrationStatus: %s\r\n", LTE_regstate2str(pvt->lte_reg_status));
			astman_append (s, "RSSI: %d, %s\r\n", pvt->rssi, rssi2dBm(pvt->rssi, buf, sizeof(buf)));
			astman_append (s, "Mode: %s\r\n", sys_mode2str(pvt->linkmode));
			astman_append (s, "Submode: %s\r\n", sys_submode2str(pvt->linksubmode));
			astman_append (s, "ProviderName: %s\r\n", pvt->provider_name);
			astman_append (s, "LocationAreaCode: %s\r\n", pvt->location_area_code);
			astman_append (s, "CellID: %s\r\n", pvt->cell_id);
			astman_append (s, "SubscriberNumber: %s\r\n", pvt->subscriber_number);
			astman_append (s, "SMSServiceCenter: %s\r\n", pvt->sms_scenter);
			astman_append (s, "UseUCS2Encoding: %s\r\n", pvt->use_ucs2_encoding ? "Yes" : "No");
			astman_append (s, "TasksInQueue: %u\r\n", PVT_STATE(pvt, at_tasks));
			astman_append (s, "CommandsInQueue: %u\r\n", PVT_STATE(pvt, at_cmds));
			astman_append (s, "CallWaitingState: %s\r\n", pvt->has_call_waiting ? "Enabled" : "Disabled");
			astman_append (s, "CurrentDeviceState: %s\r\n", dev_state2str(pvt->current_state));
			astman_append (s, "DesiredDeviceState: %s\r\n", dev_state2str(pvt->desired_state));
			astman_append (s, "CallsChannels: %u\r\n", PVT_STATE(pvt, chansno));
			astman_append (s, "Active: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_ACTIVE]));
			astman_append (s, "Held: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_ONHOLD]));
			astman_append (s, "Dialing: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_DIALING]));
			astman_append (s, "Alerting: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_ALERTING]));
			astman_append (s, "Incoming: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_INCOMING]));
			astman_append (s, "Waiting: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_WAITING]));
			astman_append (s, "Releasing: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_RELEASED]));
			astman_append (s, "Initializing: %u\r\n", PVT_STATE(pvt, chan_count[CALL_STATE_INIT]));
/* TODO: stats */

			astman_append (s, "\r\n");
			count++;
		}
		ast_mutex_unlock (&pvt->lock);
	}
	AST_RWLIST_UNLOCK (&gpublic->devices);

	astman_append (s, "Event: QuectelShowDevicesComplete\r\n");
	if(!ast_strlen_zero (id))
		astman_append (s, "ActionID: %s\r\n", id);
	astman_append (s,
		"EventList: Complete\r\n"
		"ListItems: %zu\r\n"
		"\r\n",
		count
	);

	return 0;
}

static int manager_send_ussd (struct mansession* s, const struct message* m)
{
	const char*	device	= astman_get_header (m, "Device");
	const char*	ussd	= astman_get_header (m, "USSD");

	char		buf[256];

	if (ast_strlen_zero (device))
	{
		astman_send_error (s, m, "Device not specified");
		return 0;
	}

	if (ast_strlen_zero (ussd))
	{
		astman_send_error (s, m, "USSD not specified");
		return 0;
	}

	int res = send_ussd(device, ussd);
	snprintf(buf, sizeof (buf), "[%s] %s", device, res < 0 ? error2str(chan_quectel_err) : "USSD queued for send");
	(res == 0 ? astman_send_ack : astman_send_error)(s, m, buf);

	return 0;
}

static int manager_send_sms (struct mansession* s, const struct message* m)
{
	const char*	device	= astman_get_header (m, "Device");
	const char*	number	= astman_get_header (m, "Number");
	const char*	message	= astman_get_header (m, "Message"); /* may contain C-escapes */
	const char*	validity= astman_get_header (m, "Validity");
	const char*	report	= astman_get_header (m, "Report");
	const char*	payload	= astman_get_header (m, "Payload");

	char		buf[256];

	if (ast_strlen_zero (device))
	{
		astman_send_error (s, m, "Device not specified");
		return 0;
	}

	if (ast_strlen_zero (number))
	{
		astman_send_error (s, m, "Number not specified");
		return 0;
	}

	if (ast_strlen_zero (message))
	{
		astman_send_error (s, m, "Message not specified");
		return 0;
	}

	char* unescaped_msg = ast_strdup(message);

	if (unescaped_msg == NULL)
	{
		astman_send_error (s, m, "Internal memory error");
		return 0;
	}

	ast_unescape_c(unescaped_msg);
	int res = send_sms(device, number, unescaped_msg, validity, report, payload, strlen(payload) + 1);
	ast_free(unescaped_msg);
	snprintf(buf, sizeof (buf), "[%s] %s", device, res < 0 ? error2str(chan_quectel_err) : "SMS queued for send");
	(res == 0 ? astman_send_ack : astman_send_error)(s, m, buf);

	return 0;
}

#/* */
EXPORT_DEF void manager_event_report(const char * devname, const char *payload, size_t payload_len, const char *scts, const char *dt, int success, int type, const char *report_str)
{
	manager_event (EVENT_FLAG_CALL, "QuectelReport",
		"Device: %s\r\n"
		"Payload: %.*s\r\n"
		"SCTS: %s\r\n"
		"DT: %s\r\n"
		"Success: %d\r\n"
		"Type: %d\r\n"
		"Report: %s\r\n",
		devname,
		(int) payload_len, payload,
		scts, dt,
		success,
		type,
		report_str
	);
}

/*!
 * \brief Send a QuectelNewUSSD event to the manager
 * This function splits the message in multiple lines, so multi-line
 * USSD messages can be send over the manager API.
 * \param pvt a pvt structure
 * \param message a null terminated buffer containing the message
 */

EXPORT_DEF void manager_event_new_ussd (const char * devname, char* message)
{
	struct ast_str*	buf;
	char*		s = message;
	char*		sl;
	size_t		linecount = 0;

	buf = ast_str_create (256);

	while ((sl = strsep (&s, "\r\n")))
	{
		if (*sl != '\0')
		{
			ast_str_append (&buf, 0, "MessageLine%zu: %s\r\n", linecount, sl);
			linecount++;
		}
	}

	manager_event (EVENT_FLAG_CALL, "QuectelNewUSSD",
		"Device: %s\r\n"
		"LineCount: %zu\r\n"
/* FIXME: empty lines inserted */
//		"%s\r\n",
		"%s",
		devname, linecount, ast_str_buffer (buf)
	);

	ast_free (buf);
}


#/* */
EXPORT_DEF void manager_event_message(const char * event, const char * devname, const char * message)
{
	char * escaped = espace_newlines(message);
	if(escaped) {
		manager_event_message_raw(event, devname, escaped);
		ast_free(escaped);
	}
}

#/* */
EXPORT_DEF void manager_event_message_raw(const char * event, const char * devname, const char * message)
{
	manager_event (EVENT_FLAG_CALL, event,
		"Device: %s\r\n"
		"Message: %s\r\n",
		devname,
		message
	);
}

#/* */
static char * espace_newlines(const char * text)
{
	char * escaped;
	int i, j;
	for(j = i = 0; text[i]; ++i, ++j) {
		if(text[i] == '\r' || text[i] == '\n')
			j++;
	}
	escaped = ast_malloc(j + 1);
	if(escaped) {
		for(j = i = 0; text[i]; ++i) {
			if(text[i] == '\r') {
				escaped[j++] = '\\';
				escaped[j++] = 'r';
			}
			else if(text[i] == '\n') {
				escaped[j++] = '\\';
				escaped[j++] = 'n';
			} else {
				escaped[j++] = text[i];
			}
		}
		escaped[j] = 0;
	}

	return escaped;
}

#/* */
EXPORT_DEF void manager_event_cend(const char * devname, int call_index, int duration, int end_status, int cc_cause)
{
	manager_event( EVENT_FLAG_CALL, "QuectelCEND",
		"Device: %s\r\n"
		"CallIdx: %d\r\n"
		"Duration: %d\r\n"
		"EndStatus: %d\r\n"
		"CCCause: %d\r\n",
		devname,
		call_index,
		duration,
		end_status,
		cc_cause
		);
}

#/* */
EXPORT_DEF void manager_event_call_state_change(const char * devname, int call_index, const char * newstate)
{
	manager_event(EVENT_FLAG_CALL, "QuectelCallStateChange",
		"Device: %s\r\n"
		"CallIdx: %d\r\n"
		"NewState: %s\r\n",
		devname,
		call_index,
		newstate
		);
}

#/* */
EXPORT_DEF void manager_event_device_status(const char * devname, const char * newstate)
{
	manager_event(EVENT_FLAG_CALL, "QuectelStatus",
		"Device: %s\r\n"
		"Status: %s\r\n",
		devname,
		newstate
		);
}


/*!
 * \brief Send a QuectelNewSMS event to the manager
 * This function splits the message in multiple lines, so multi-line
 * SMS messages can be send over the manager API.
 * \param pvt a pvt structure
 * \param number a null terminated buffer containing the from number
 * \param message a null terminated buffer containing the message
 */

// TODO: use espace_newlines() and join with manager_event_new_sms_base64()
EXPORT_DEF void manager_event_new_sms (const char * devname, char* number, char* message)
{
	struct ast_str* buf;
	size_t	linecount = 0;
	char*	s = message;
	char*	sl;

	buf = ast_str_create (256);

	while ((sl = strsep (&s, "\r\n")))
	{
		if (*sl != '\0')
		{
			ast_str_append (&buf, 0, "MessageLine%zu: %s\r\n", linecount, sl);
			linecount++;
		}
	}

	manager_event (EVENT_FLAG_CALL, "QuectelNewSMS",
		"Device: %s\r\n"
		"From: %s\r\n"
		"LineCount: %zu\r\n"
		"%s\r\n",
		devname, number, linecount, ast_str_buffer (buf)
	);
	ast_free (buf);
}

/*!
 * \brief Send a QuectelNewSMSBase64 event to the manager
 * \param pvt a pvt structure
 * \param number a null terminated buffer containing the from number
 * \param message_base64 a null terminated buffer containing the base64 encoded message
 */

EXPORT_DEF void manager_event_new_sms_base64 (const char * devname, char * number, char * message_base64)
{
	manager_event (EVENT_FLAG_CALL, "QuectelNewSMSBase64",
		"Device: %s\r\n"
		"From: %s\r\n"
		"Message: %s\r\n",
		devname, number, message_base64
	);
}

static int manager_ccwa_set (struct mansession* s, const struct message* m)
{
	const char*	device	= astman_get_header (m, "Device");
	const char*	value	= astman_get_header (m, "Value");
//	const char*	id	= astman_get_header (m, "ActionID");

	char		buf[256];
	call_waiting_t	enable;

	if (ast_strlen_zero (device))
	{
		astman_send_error (s, m, "Device not specified");
		return 0;
	}

	if (strcmp("enable", value) == 0)
		enable = CALL_WAITING_ALLOWED;
	else if (strcmp("disable", value) == 0)
		enable = CALL_WAITING_DISALLOWED;
	else
	{
		astman_send_error (s, m, "Invalid Value");
		return 0;
	}

	int res = send_ccwa_set(device, enable);
	snprintf (buf, sizeof (buf), "[%s] %s", device, res < 0 ? error2str(chan_quectel_err) : "Call-Waiting commands queued for execute");
	(res == 0 ? astman_send_ack : astman_send_error)(s, m, buf);

//	if(!ast_strlen_zero(id))
//		astman_append (s, "ActionID: %s\r\n", id);

	return 0;
}

static int manager_reset (struct mansession* s, const struct message* m)
{
	const char*	device	= astman_get_header (m, "Device");
//	const char*	id	= astman_get_header (m, "ActionID");

	char		buf[256];

	if (ast_strlen_zero (device))
	{
		astman_send_error (s, m, "Device not specified");
		return 0;
	}

	int res = send_reset(device);
	snprintf (buf, sizeof (buf), "[%s] %s", device, res < 0 ? error2str(chan_quectel_err) : "Reset command queued for execute");
	(res == 0 ? astman_send_ack : astman_send_error)(s, m, buf);

//	if(!ast_strlen_zero(id))
//		astman_append (s, "ActionID: %s\r\n", id);

	return 0;
}

static const char * const b_choices[] = { "now", "gracefully", "when convenient" };
#/* */
static int manager_restart_action(struct mansession * s, const struct message * m, dev_state_t event)
{

	const char * device = astman_get_header (m, "Device");
	const char * when = astman_get_header (m, "When");
//	const char * id = astman_get_header (m, "ActionID");

	char buf[256];
	int res;
	unsigned i;

	if (ast_strlen_zero (device))
	{
		astman_send_error (s, m, "Device not specified");
		return 0;
	}

	for(i = 0; i < ITEMS_OF(b_choices); i++)
	{
		if(event == DEV_STATE_STARTED || strcasecmp(when, b_choices[i]) == 0)
		{
			res = schedule_restart_event(event, i, device);
			snprintf(buf, sizeof (buf), "[%s] %s", device, res < 0 ? error2str(chan_quectel_err) : dev_state2str_msg(event));
			(res == 0 ? astman_send_ack : astman_send_error)(s, m, buf);
//			if(!ast_strlen_zero(id))
//				astman_append (s, "ActionID: %s\r\n", id);
			return 0;
		}
	}

	astman_send_error (s, m, "Invalid value of When");
//	if(!ast_strlen_zero(id))
//		astman_append (s, "ActionID: %s\r\n", id);
	return 0;
}

#/* */
static int manager_restart(struct mansession * s, const struct message * m)
{
	return manager_restart_action(s, m, DEV_STATE_RESTARTED);
}

#/* */
static int manager_stop(struct mansession * s, const struct message * m)
{
	return manager_restart_action(s, m, DEV_STATE_STOPPED);
}

#/* */
static int manager_start(struct mansession * s, const struct message * m)
{
	return manager_restart_action(s, m, DEV_STATE_STARTED);
}

#/* */
static int manager_remove(struct mansession * s, const struct message * m)
{
	return manager_restart_action(s, m, DEV_STATE_REMOVED);
}

#/* */
static int manager_reload(struct mansession * s, const struct message * m)
{
	const char * when = astman_get_header (m, "When");
//	const char * id = astman_get_header (m, "ActionID");

	unsigned i;

	for(i = 0; i < ITEMS_OF(b_choices); i++)
	{
		if(strcasecmp(when, b_choices[i]) == 0)
		{
			pvt_reload(i);
			astman_send_ack(s, m, "reload scheduled");
//			if(!ast_strlen_zero(id))
//				astman_append (s, "ActionID: %s\r\n", id);
			return 0;
		}
	}

	astman_send_error (s, m, "Invalid value of When");
//	if(!ast_strlen_zero(id))
//		astman_append (s, "ActionID: %s\r\n", id);
	return 0;
}

static const struct quectel_manager
{
	int		(*func)(struct mansession* s, const struct message* m);
	int		authority;
	const char*	name;
	const char*	brief;
	const char*	desc;
} dcm[] =
{
	{
	manager_show_devices,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_REPORTING,
	"QuectelShowDevices",
	"List Quectel devices",
	"Description: Lists Quectel devices in text format with details on current status.\n\n"
	"QuectelShowDevicesComplete.\n"
	"Variables:\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	Device:   <name>	Optional name of device.\n"
	},
	{
	manager_send_ussd,
	EVENT_FLAG_CALL,
	"QuectelSendUSSD",
	"Send a ussd command to the quectel.",
	"Description: Send a ussd message to a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel to which the ussd code will be sent.\n"
	"	*USSD:    <code>	The ussd code that will be sent to the device.\n"
	 },
	{
	manager_send_sms,
	EVENT_FLAG_CALL,
	"QuectelSendSMS",
	"Send a SMS message.",
	"Description: Send a SMS message from a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel to which the SMS be send.\n"
	"	*Number:  <number>	The phone number to which the SMS will be sent.\n"
	"	*Message: <message>	The SMS message that will be sent (standard backslash escape sequences are used, e.g. '\\n' for newline).\n"
	"	*Validity: <message>	Validity period in minutes.\n"
	"	*Report: <message>	Boolean flag for report request.\n"
	"	*Payload: <message>	Unstructured data that will be included in delivery report.\n"
	},
	{
	manager_ccwa_set,
	EVENT_FLAG_CONFIG,
	"QuectelSetCCWA",
	"Enable/Disabled Call-Waiting on a quectel.",
	"Description: Enable/Disabled Call-Waiting on a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel to which the command be send.\n"
	"	*Value:   <enable|disable> Value of Call Waiting\n"
	 },
	{
	manager_reset,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_CONFIG,
	"QuectelReset",
	"Reset a quectel.",
	"Description: Reset a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel which should be reset.\n"
	},
	{
	manager_restart,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_CONFIG,
	"QuectelRestart",
	"Restart a quectel.",
	"Description: Restart a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel which should be restart.\n"
	"	*When:    < now | gracefully | when convenient > Time when device restarted.\n"
	},
	{
	manager_stop,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_CONFIG,
	"QuectelStop",
	"Stop a quectel.",
	"Description: Stop a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel which should be stopped.\n"
	"	*When:    < now | gracefully | when convenient > Time when device stopped.\n"
	},
	{
	manager_start,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_CONFIG,
	"QuectelStart",
	"Start a quectel.",
	"Description: Start a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel which should be started.\n"
	},
	{
	manager_remove,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_CONFIG,
	"QuectelRemove",
	"Remove a quectel.",
	"Description: Remove a quectel.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*Device:  <device>	The quectel which should be removed.\n"
	"	*When:    < now | gracefully | when convenient > Time when device removed.\n"
	},
	{
	manager_reload,
	EVENT_FLAG_SYSTEM | EVENT_FLAG_CONFIG,
	"QuectelReload",
	"Reload a module configuration.",
	"Description: Reload the module configuration.\n\n"
	"Variables: (Names marked with * are required)\n"
	"	ActionID: <id>		Action ID for this transaction. Will be returned.\n"
	"	*When:    < now | gracefully | when convenient > Time when devices reconfigured.\n"
	},
};

EXPORT_DEF void manager_register()
{
	unsigned i;
#if ASTERISK_VERSION_NUM >= 130000 /* 13+ */
	struct ast_module* module = self_module();
#endif /* ^13+ */

	for(i = 0; i < ITEMS_OF(dcm); i++)
	{
#if ASTERISK_VERSION_NUM >= 130000 /* 13+ */
		ast_manager_register2 (dcm[i].name, dcm[i].authority, dcm[i].func,
			module, dcm[i].brief, dcm[i].desc);
#elif ASTERISK_VERSION_NUM >= 110000 /* 11+ */
		ast_manager_register2 (dcm[i].name, dcm[i].authority, dcm[i].func, NULL, dcm[i].brief, dcm[i].desc);
#else /* 11- */
		ast_manager_register2 (dcm[i].name, dcm[i].authority, dcm[i].func, dcm[i].brief, dcm[i].desc);
#endif /* ^11- */
	}
}

EXPORT_DEF void manager_unregister()
{
	int i;
	for(i = ITEMS_OF(dcm)-1; i >= 0; i--)
	{
		ast_manager_unregister((char*)dcm[i].name);
	}
}

#endif /* BUILD_MANAGER */
