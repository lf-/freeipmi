/*
ipmi-chassis.c: IPMI Chassis control.
Copyright (C) 2007 FreeIPMI Core Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <argp.h>
#include <assert.h>

#include <freeipmi/freeipmi.h>
#include <freeipmi/udm/udm.h>

#include "argp-common.h"
#include "fiid-wrappers.h"
#include "freeipmi-portability.h"
#include "ipmi-chassis.h"
#include "ipmi-chassis-argp.h"
#include "tool-common.h"

#include "pstdout.h"
#include "hostrange.h"

#define _FIID_OBJ_GET_WITH_RETURN_VALUE(obj, field, val, rv)            \
do {                                                                    \
    uint64_t _val = 0, *_val_ptr;                                       \
    _val_ptr = val;                                                     \
    if ((rv = fiid_obj_get (obj, field, &_val)) < 0)                    \
      {                                                                 \
        pstdout_fprintf(state_data->pstate,                             \
                        stderr,                                         \
                        "fiid_obj_get: %s: %s\n",                       \
                        field,                                          \
                        fiid_strerror(fiid_obj_errnum(obj)));           \
        goto cleanup;                                                   \
      }                                                                 \
    *_val_ptr = _val;                                                   \
} while (0)

#define _FIID_OBJ_GET(obj, field, val)                                  \
do {                                                                    \
    uint64_t _val = 0, *_val_ptr;                                       \
    _val_ptr = val;                                                     \
    if (fiid_obj_get (obj, field, &_val) < 0)                           \
      {                                                                 \
        pstdout_fprintf(state_data->pstate,                             \
                        stderr,                                         \
                        "fiid_obj_get: %s: %s\n",                       \
                        field,                                          \
                        fiid_strerror(fiid_obj_errnum(obj)));           \
        goto cleanup;                                                   \
      }                                                                 \
    *_val_ptr = _val;                                                   \
} while (0)

static int32_t 
set_boot_flags (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  fiid_obj_t boot_info_ack_cmd_rs = NULL; 
  fiid_obj_t get_boot_flags_rs = NULL;
  uint8_t boot_info_acknowledge = IPMI_CHASSIS_BOOT_OPTIONS_BOOT_INFO_UNACKNOWLEDGE;
  uint8_t bios_boot_type, boot_flags_persistent, boot_flags_valid, 
    lock_out_reset_button, screen_blank, boot_device_selector, 
    lock_keyboard, clear_cmos, console_redirection, lock_out_sleep_button, 
    user_password_bypass, force_progress_event_traps, firmware_bios_verbosity, 
    lock_out_via_power_button, bios_mux_control_override, bios_shared_mode_override;
  uint64_t val =0;
  int32_t rv = -1;
  struct ipmi_chassis_arguments *args;

  args = state_data->prog_data->args;

  if (!(get_boot_flags_rs = fiid_obj_create (tmpl_cmd_get_system_boot_options_boot_flags_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      goto cleanup;
    }
      
  if (!(boot_info_ack_cmd_rs = fiid_obj_create (tmpl_cmd_set_system_boot_options_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      goto cleanup;
    }

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_set_system_boot_options_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      goto cleanup;
    }

  if (ipmi_cmd_get_system_boot_options_boot_flags ( state_data->dev, 
                                                  IPMI_CHASSIS_BOOT_OPTIONS_NO_SET_SELECTOR, 
                                                  IPMI_CHASSIS_BOOT_OPTIONS_NO_BLOCK_SELECTOR,
                                                  get_boot_flags_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_get_system_boot_options_boot_flags: %s\n", 
                      ipmi_device_strerror(ipmi_device_errnum(state_data->dev)));
      goto cleanup;
    }

  if (args->args.boot_option_args.bios_boot_type == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "bios_boot_type", &val);
      bios_boot_type = val;
    }
  else
    bios_boot_type = args->args.boot_option_args.bios_boot_type;

  boot_flags_persistent = IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_VALID_FOR_NEXT_BOOT;
  boot_flags_valid = IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_VALID;

  if (args->args.boot_option_args.lock_out_reset_button == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "lock_out_reset_button", &val);
      lock_out_reset_button = val;
    }
  else
    lock_out_reset_button = args->args.boot_option_args.lock_out_reset_button;

  if (args->args.boot_option_args.screen_blank == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "screen_blank", &val);
      screen_blank = val;
    }
  else
    screen_blank = args->args.boot_option_args.screen_blank;

  if (args->args.boot_option_args.boot_device_selector == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "boot_device_selector", &val);
      boot_device_selector = val;
    }
  else
    boot_device_selector = args->args.boot_option_args.boot_device_selector;

  if (args->args.boot_option_args.lock_keyboard == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "lock_keyboard", &val);
      lock_keyboard = val;
    }
  else
    lock_keyboard = args->args.boot_option_args.lock_keyboard;

  if (args->args.boot_option_args.clear_cmos == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "clear_cmos", &val);
      clear_cmos = val;
    }
  else
    clear_cmos = args->args.boot_option_args.clear_cmos;

  if (args->args.boot_option_args.console_redirection == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "console_redirection", &val);
      console_redirection = val;
    }
  else
    console_redirection = args->args.boot_option_args.console_redirection;

  _FIID_OBJ_GET (get_boot_flags_rs, "lock_out_sleep_button", &val);
  lock_out_sleep_button = val;

  if (args->args.boot_option_args.user_password_bypass == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "user_password_bypass", &val);
      user_password_bypass = val;
    }
  else
    user_password_bypass = args->args.boot_option_args.user_password_bypass;

  if (args->args.boot_option_args.force_progress_event_traps == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "force_progress_event_traps", &val);
      force_progress_event_traps = val;
    }
  else
    force_progress_event_traps = args->args.boot_option_args.force_progress_event_traps;

  if (args->args.boot_option_args.firmware_bios_verbosity == -1)
    {
      _FIID_OBJ_GET (get_boot_flags_rs, "firmware_bios_verbosity", &val);
      firmware_bios_verbosity = val;
    }
  else
    firmware_bios_verbosity = args->args.boot_option_args.firmware_bios_verbosity;

  _FIID_OBJ_GET (get_boot_flags_rs, "lock_out_via_power_button", &val);
  lock_out_via_power_button = val;

  _FIID_OBJ_GET (get_boot_flags_rs, "bios_mux_control_override", &val);
  bios_mux_control_override = val;

  _FIID_OBJ_GET (get_boot_flags_rs, "bios_shared_mode_override", &val);
  bios_shared_mode_override = val;

  if (ipmi_cmd_set_system_boot_options_boot_flags (state_data->dev, 
                                                 bios_boot_type,
                                                 boot_flags_persistent,
                                                 boot_flags_valid,
                                                 lock_out_reset_button,
                                                 screen_blank,
                                                 boot_device_selector,
                                                 lock_keyboard,
                                                 clear_cmos,
                                                 console_redirection,
                                                 lock_out_sleep_button,
                                                 user_password_bypass,
                                                 force_progress_event_traps,
                                                 firmware_bios_verbosity,
                                                 lock_out_via_power_button,
                                                 bios_mux_control_override,
                                                 bios_shared_mode_override,
                                                 cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_set_sytem_boot_option_boot_flags failed: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  if (ipmi_cmd_set_system_boot_options_boot_info_acknowledge (state_data->dev,
                                                              &boot_info_acknowledge,
                                                              &boot_info_acknowledge,
                                                              &boot_info_acknowledge,
                                                              &boot_info_acknowledge,
                                                              &boot_info_acknowledge,
                                                              boot_info_ack_cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_set_system_boot_options_boot_info_acknowledge: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (get_boot_flags_rs);
  FIID_OBJ_DESTROY_NO_RETURN (boot_info_ack_cmd_rs);
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

static int32_t 
get_boot_flags (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  uint64_t val = 0;
  int32_t rv = -1;
  char tmp[256];

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_get_system_boot_options_boot_flags_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return -1;
    }
  
  if (ipmi_cmd_get_system_boot_options_boot_flags ( state_data->dev, 
                                                  IPMI_CHASSIS_BOOT_OPTIONS_NO_SET_SELECTOR, 
                                                  IPMI_CHASSIS_BOOT_OPTIONS_NO_BLOCK_SELECTOR,
                                                  cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_get_system_boot_options_boot_flags failed: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  _FIID_OBJ_GET (cmd_rs, "bios_boot_type", &val);
  sprintf (tmp, "BIOS boot type                : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_TYPE_PC_COMPATIBLE:
        strcat (tmp, "PC compatible boot");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_TYPE_EFI:
        strcat (tmp, "Extensible firmware Interface boot");
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "lock_out_reset_button", &val);
  sprintf (tmp, "Lock out reset buttons        : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "screen_blank", &val);
  sprintf (tmp, "Screen blank                  : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;
      
      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "boot_device_selector", &val);
  sprintf (tmp, "Boot device selector          : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_NONE:
        strcat (tmp, "None");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_PXE:
        strcat (tmp, "Force PXE");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_DISK:
        strcat (tmp, "Force boot from default Hard drive");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_DISK_SAFE_MODE:
        strcat (tmp, "Force boot from default Hard drive, request safe mode");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_BIOS:
        strcat (tmp, "Force boot into BIOS setup");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_CD_DVD:
        strcat (tmp, "Force boot from default CD/DVD");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_DIAGNOSTIC_INTERRUPT:
        strcat (tmp, "Force boot from default Diagnostic partition");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_BOOT_DEVICE_FLOPPY:
        strcat (tmp, "Force boot from default Floppy/primary removable media");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "lock_keyboard", &val);
  sprintf (tmp, "Lock keyboard                 : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "clear_cmos", &val);
  sprintf (tmp, "Clear CMOS                    : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "console_redirection", &val);
  sprintf (tmp, "Console redirection control   : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_CONSOLE_REDIRECTION_DEFAULT:
        strcat (tmp, "System default");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_CONSOLE_REDIRECTION_SUPRESS:
        strcat (tmp, "Suppressed");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_CONSOLE_REDIRECTION_ENABLE:
        strcat (tmp, "Enabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);
  
  _FIID_OBJ_GET (cmd_rs, "lock_out_sleep_button", &val);
  sprintf (tmp, "Lock out sleep button         : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "user_password_bypass", &val);
  sprintf (tmp, "User password bypass          : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "lock_out_reset_button", &val);
  sprintf (tmp, "Lock out reset button         : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;
  
      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "force_progress_event_traps", &val);
  sprintf (tmp, "Force progress event traps    : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "firmware_bios_verbosity", &val);
  sprintf (tmp, "Firmware BIOS verbosity level : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_FIRMWARE_BIOS_VERBOSITY_QUIET:
        strcat (tmp, "Quiet");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_FIRMWARE_BIOS_VERBOSITY_VERBOSE:
        strcat (tmp, "Verbose");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_BOOT_FLAG_FIRMWARE_BIOS_VERBOSITY_DEFAULT:
        strcat (tmp, "Default");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp);

  _FIID_OBJ_GET (cmd_rs, "lock_out_via_power_button", &val);
  sprintf (tmp, "Lock out via power button     : ");
  switch (val)
    {
      case IPMI_CHASSIS_BOOT_OPTIONS_ENABLE:
        strcat (tmp, "Enabled");
        break;

      case IPMI_CHASSIS_BOOT_OPTIONS_DISABLE:
        strcat (tmp, "Disabled");
        break;
    }
  pstdout_printf (state_data->pstate, "%s\n", tmp); 
  rv = 0;
cleanup:
  FIID_OBJ_DESTROY (cmd_rs);
  return rv;
}


static int32_t
set_power_cycle_interval (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  int32_t rv = -1;
  struct ipmi_chassis_arguments *args;

  args = state_data->prog_data->args;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_set_power_cycle_interval_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return (-1);
    }

  if (ipmi_cmd_set_power_cycle_interval (state_data->dev, 
                                         args->args.power_cycle_interval, 
                                         cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_set_power_cycle_interval: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

static int32_t 
set_power_restore_policy (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  uint64_t val = 0;
  int32_t rv = -1;
  struct ipmi_chassis_arguments *args;

  args = state_data->prog_data->args;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_set_power_restore_policy_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return -1;
    }

  if (ipmi_cmd_set_power_restore_policy (state_data->dev,
                                         args->args.power_restore_policy, 
                                         cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_set_power_restore_policy: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  if (args->args.power_restore_policy == IPMI_POWER_RESTORE_POLICY_NO_CHANGE)
    {
      char policy_supported[100];
      memset(policy_supported, 0, sizeof (policy_supported));
  
      _FIID_OBJ_GET (cmd_rs, "powered_off_after_ac_mains_returns", &val);
      if (val)
        sprintf (policy_supported, "always-off ");  
  
      _FIID_OBJ_GET (cmd_rs, "always_powering_up_after_ac_mains_returns", &val);
      if (val)
        strcat (policy_supported, "always-on ");
  
      _FIID_OBJ_GET (cmd_rs, "restoring_power_to_state_when_ac_mains_was_lost", &val);
      if (val)
        strcat (policy_supported, "Restore");
  
      pstdout_printf (state_data->pstate, 
                      "Policies supported          : %s\n", 
                      policy_supported);
    }

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}


static int32_t
get_power_on_hours_counter (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  uint64_t val = 0;
  uint8_t minutes_per_counter;
  uint64_t counts;
  uint32_t min, hrs;
  int32_t rv = -1;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_get_power_on_hours_counter_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return -1;
    }
  
  if (ipmi_cmd_get_power_on_hours_counter (state_data->dev, cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_get_power_on_hours_counter: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  _FIID_OBJ_GET (cmd_rs, "minutes_per_counter", &val);
  minutes_per_counter = val;

  _FIID_OBJ_GET (cmd_rs, "counter_reading", &val);
  counts = val; 

  min = counts / minutes_per_counter;
  hrs = min / 60;
  min = min % 60;

  pstdout_printf (state_data->pstate, 
                  "Power on hours             : %d Hours %d Minutes\n", 
                  hrs, 
                  min);
  rv = 0;

cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

static int32_t
get_system_restart_cause (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  uint64_t val = 0;
  char restart_cause[256];
  int32_t rv = -1;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_get_system_restart_cause_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return -1;
    }

  if (ipmi_cmd_get_system_restart_cause (state_data->dev, cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_get_system_restart_cause: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  _FIID_OBJ_GET (cmd_rs, "restart_cause", &val);
  switch (val)
    {
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_UNKNOWN:
        sprintf (restart_cause,"Unknown\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_CHASSIS_CONTROL_CMD:
        sprintf (restart_cause, "Chassis control command\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_RESET_PUSHBUTTON:
        sprintf (restart_cause, "Reset via pushbutton\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_POWER_UP_POWER_PUSHBUTTON:
        sprintf (restart_cause, "Power up via power pushbutton\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_WATCHDOG_EXPIRE:
        sprintf (restart_cause, "Watchdog expiration\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_OEM:
        sprintf (restart_cause, "OEM\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_AUTO_POWER_UP_ALWAYS_RESTORE:
        sprintf (restart_cause, "Automatic power-up on AC being applied due to \"always restore\" power restore policy\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_AUTO_POWER_UP_RESTORE_PREVIOUS:
        sprintf (restart_cause, "Automatic power-up on AC being applied due to \"restore previous power state\" power restore policy\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_RESET_VIA_PEF:
        sprintf (restart_cause, "Reset via PEF\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_POWER_CYCLE_VIA_PEF:
        sprintf (restart_cause, "Power cycle via PEF\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_SOFT_RESET:
        sprintf (restart_cause, "Soft reset\n");
        break;
  
      case IPMI_CHASSIS_SYSTEM_RESTART_CAUSE_POWER_UP_VIA_RTC:
        sprintf (restart_cause, "Power up via RTC\n");
        break;
    }

  pstdout_printf (state_data->pstate, 
                  "Restart cause              : %s\n", 
                  restart_cause);

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

static int32_t
chassis_identify (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  int32_t rv = -1;
  struct ipmi_chassis_arguments *args;

  args = state_data->prog_data->args;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_chassis_identify_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return (-1);
    }

  if (ipmi_cmd_chassis_identify (state_data->dev, 
                                 (args->args.identify_args.identify_interval_set) ? &args->args.identify_args.identify_interval : NULL, 
                                 (args->args.identify_args.force_identify_set) ? &args->args.identify_args.force_identify : NULL, 
                                 cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_chassis_identify: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}
  

static int32_t 
chassis_control (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  int32_t rv = -1;
  struct ipmi_chassis_arguments *args;

  args = state_data->prog_data->args;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_chassis_control_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return -1;
    }

  if (ipmi_cmd_chassis_control (state_data->dev, 
                                args->args.chassis_control, 
                                cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_chassis_control: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

static int32_t 
get_chassis_status (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  uint64_t val = 0, temp_val;
  uint8_t front_panel_capabilities = 0, misc_chassis_status = 0;
  int32_t rv = -1;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_get_chassis_status_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      goto cleanup;
    }

  if (ipmi_cmd_get_chassis_status (state_data->dev, cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_get_chassis_status: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  _FIID_OBJ_GET (cmd_rs, "current_power_state.power_is_on", &val);
  pstdout_printf (state_data->pstate, 
                  "System Power               : %s\n", 
                  val ? "on" : "off");

  _FIID_OBJ_GET (cmd_rs, "current_power_state.power_overload", &val);
  pstdout_printf (state_data->pstate, 
                  "System Power Overload      : %s\n", 
                  val ? "true" : "false");
 
  _FIID_OBJ_GET (cmd_rs, "current_power_state.interlock", &val);
  pstdout_printf (state_data->pstate, 
                  "Interlock switch           : %s\n", 
                  val ? "active" : "Inactive");

  _FIID_OBJ_GET (cmd_rs, "current_power_state.power_fault", &val);
  pstdout_printf (state_data->pstate, 
                  "Power fault detected       : %s\n", 
                  val ? "true" : "false");

  _FIID_OBJ_GET (cmd_rs, "current_power_state.power_control_fault", &val);
  pstdout_printf (state_data->pstate, 
                  "Power control fault        : %s\n", 
                  val ? "true" : "false");

  _FIID_OBJ_GET (cmd_rs, "current_power_state.power_restore_policy", &val);
  pstdout_printf (state_data->pstate, 
                  "Power restore policy       :");

  switch (val)
    {
      case IPMI_POWER_RESTORE_POLICY_POWERED_OFF_AFTER_AC_RETURNS: 
        pstdout_printf (state_data->pstate, " Always off\n");
        break;

      case IPMI_POWER_RESTORE_POLICY_POWER_RESTORED_TO_STATE: 
        pstdout_printf (state_data->pstate, " Restore\n");
        break;
  
      case IPMI_POWER_RESTORE_POLICY_POWERS_UP_AFTER_AC_RETURNS:
        pstdout_printf (state_data->pstate, " Always on\n");
        break;

      case IPMI_POWER_RESTORE_POLICY_UNKNOWN: 
        pstdout_printf (state_data->pstate, " Unknown\n");
        break;
    }

  temp_val = IPMI_LAST_POWER_EVENT_UNKNOWN;
  _FIID_OBJ_GET (cmd_rs, "last_power_event.ac_failed", &val);
  if (val)
    {
      temp_val = IPMI_LAST_POWER_EVENT_AC_FAILED;
      goto print;
    }

  _FIID_OBJ_GET (cmd_rs, "last_power_event.power_down_caused_by_power_overload", &val);
  if (val)
    {
      temp_val = IPMI_LAST_POWER_EVENT_POWER_DOWN_POWER_OVERLOAD;
      goto print;
    }

  _FIID_OBJ_GET (cmd_rs, "last_power_event.power_down_caused_by_power_interlock_being_activated", &val);
  if (val)
    {
      temp_val = IPMI_LAST_POWER_EVENT_POWER_DOWN_INTERLOCK_ACTIVATED;
      goto print;
    }

  _FIID_OBJ_GET (cmd_rs, "last_power_event.power_down_caused_by_power_fault", &val);
  if (val)
    {
      temp_val = IPMI_LAST_POWER_EVENT_POWER_DOWN_POWER_FAULT;
      goto print;
    }

  _FIID_OBJ_GET (cmd_rs, "last_power_event.power_on_entered_via_ipmi", &val);
  if (val)
      temp_val = IPMI_LAST_POWER_EVENT_POWER_ON_VIA_IPMI;

print:
  pstdout_printf (state_data->pstate, "Last Power Event           : ");
  switch (temp_val)
    {
      case IPMI_LAST_POWER_EVENT_AC_FAILED:
        pstdout_printf (state_data->pstate, "ac failed\n");
        break;

      case IPMI_LAST_POWER_EVENT_POWER_DOWN_POWER_OVERLOAD:
        pstdout_printf (state_data->pstate, "power down due to power overload\n");
        break;

      case IPMI_LAST_POWER_EVENT_POWER_DOWN_INTERLOCK_ACTIVATED:
        pstdout_printf (state_data->pstate, "power down due to Activation of interlock switch\n");
        break;

      case IPMI_LAST_POWER_EVENT_POWER_DOWN_POWER_FAULT:
        pstdout_printf (state_data->pstate, "power down due to power fault\n");
        break;

      case IPMI_LAST_POWER_EVENT_POWER_ON_VIA_IPMI:
        pstdout_printf (state_data->pstate, "power on via ipmi command\n");
        break;

      default:
        pstdout_printf (state_data->pstate, "unknown power event\n");
        break;
    }

  pstdout_printf (state_data->pstate, "Misc Chassis status        :");
  _FIID_OBJ_GET (cmd_rs, "misc_chassis_state.chassis_intrusion_active", &val);
  if (val)
    {
      misc_chassis_status = 1;
      pstdout_printf (state_data->pstate, " Chassis Intrusion Active");
    }

  _FIID_OBJ_GET (cmd_rs,  "misc_chassis_state.front_panel_lockout_active", &val);
  if (val) 
    {
      if (misc_chassis_status)
        pstdout_printf (state_data->pstate, 
                        "\n                            ");

      misc_chassis_status = 1;
      pstdout_printf (state_data->pstate, 
                      " Front panel lockout active");
    }

  _FIID_OBJ_GET (cmd_rs, "misc_chassis_state.drive_fault", &val);
  if (val)
    {
      if (misc_chassis_status)
        pstdout_printf (state_data->pstate, 
                        "\n                            ");
      misc_chassis_status = 1;
      pstdout_printf (state_data->pstate, 
                      "                             Drive Fault");
    }

  _FIID_OBJ_GET (cmd_rs, "misc_chassis_state.cooling_fan_fault_detected", &val);
  if (val)
    {
      if (misc_chassis_status)
        pstdout_printf (state_data->pstate, 
                        "\n                            ");
      misc_chassis_status = 1;
      pstdout_printf (state_data->pstate, 
                      "                             Cooling fan fault detected");
    }

  _FIID_OBJ_GET (cmd_rs, "misc_chassis_state.chassis_identify_command_and_state_info_supported", &val);
  if (val)
    {
      if (misc_chassis_status)
        pstdout_printf (state_data->pstate, 
                        "\n                            ");
      misc_chassis_status = 1;
      pstdout_printf (state_data->pstate, 
                      "                             Chassis Identify Command and State Info supported");

      _FIID_OBJ_GET (cmd_rs, "misc_chassis_state.chassis_identify_state", &val);
      pstdout_printf (state_data->pstate, 
                      "\nChassis Identify state     : ");
      switch (val)
        {
            case IPMI_CHASSIS_IDENTIFY_STATE_OFF: 
                pstdout_printf (state_data->pstate, "off\n");
                break;
            
            case IPMI_CHASSIS_IDENTIFY_STATE_TEMPORARY_ON: 
                pstdout_printf (state_data->pstate, "Timed on\n");
                break;
        
            case IPMI_CHASSIS_IDENTIFY_STATE_INDEFINITE_ON: 
                pstdout_printf (state_data->pstate, "Indefinite on\n");
                break;
        }
    }
  else if (misc_chassis_status)
    pstdout_printf (state_data->pstate, "\n");
  
  if (!misc_chassis_status)
    pstdout_printf (state_data->pstate, "\n");
  
  rv = 0;
  pstdout_printf (state_data->pstate, 
                  "Front panel capabilities   :");
  _FIID_OBJ_GET_WITH_RETURN_VALUE (cmd_rs, "front_panel.power_off_button_disabled", &val, rv);
  if (rv)
    {
      if (val)
        {  
          front_panel_capabilities  = 1;
          pstdout_printf (state_data->pstate, 
                          " Power off button disabled");
        }
    
      _FIID_OBJ_GET (cmd_rs, "front_panel.reset_button_disabled", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");

          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          " Reset button disabled");
        }
    
      _FIID_OBJ_GET (cmd_rs, "front_panel.diagnostic_interrupt_button_disabled", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");
          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          " Diagnostic Interrupt Button disabled");
        }

      _FIID_OBJ_GET (cmd_rs, "front_panel.standy_button_disabled", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");
          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          "                            Standby button disabled");
        }
    
      _FIID_OBJ_GET (cmd_rs, "front_panel.power_off_button_disable_allowed", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");
          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          " Power off button disable allowed");
        }
     
      _FIID_OBJ_GET (cmd_rs, "front_panel.reset_button_disable_allowed", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");
          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          " Reset button disable allowed");
        }
    
      _FIID_OBJ_GET (cmd_rs, "front_panel.diagnostic_interrupt_button_disable_allowed", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");
          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          " Diagnostic interrupt button disable allowed");
        }
    
      _FIID_OBJ_GET (cmd_rs, "front_panel.standby_button_disable_allowed", &val);
      if (val)
        {
          if (front_panel_capabilities)
              pstdout_printf (state_data->pstate, 
                              "\n                            ");
          front_panel_capabilities = 1;
          pstdout_printf (state_data->pstate, 
                          " Standby button disable allowed");
        }
    }
    
  pstdout_printf (state_data->pstate, "\n");
    
  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

static int32_t
get_chassis_capabilities (ipmi_chassis_state_data_t *state_data)
{
  fiid_obj_t cmd_rs = NULL;
  int32_t rv = -1;
  uint64_t val = 0;

  if (!(cmd_rs = fiid_obj_create (tmpl_cmd_get_chassis_capabilities_rs)))
    {
      pstdout_perror(state_data->pstate, "fiid_obj_create");
      return (-1);
    }

  if (ipmi_cmd_get_chassis_capabilities (state_data->dev, cmd_rs) != 0)
    {
      pstdout_fprintf(state_data->pstate,
                      stderr,
                      "ipmi_cmd_get_chassis_capabilities: %s\n", 
                      ipmi_device_strerror (ipmi_device_errnum (state_data->dev)));
      goto cleanup;
    }

  _FIID_OBJ_GET (cmd_rs,
                 "intrusion_sensor",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "Intrusion Sensor           : %s\n",
                  (val ? "Provided" : "Not Provided"));

  _FIID_OBJ_GET (cmd_rs,
                 "front_panel_lockout",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "Front Panel Lockout        : %s\n",
                  (val ? "Provided" : "Not Provided"));

  _FIID_OBJ_GET (cmd_rs,
                 "diagnostic_interrupt",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "Diagnostic Interrupt       : %s\n",
                  (val ? "Provided" : "Not Provided"));

  _FIID_OBJ_GET (cmd_rs,
                 "power_interlock",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "Power Interlock            : %s\n",
                  (val ? "Provided" : "Not Provided"));

  _FIID_OBJ_GET (cmd_rs,
                 "fru_info_device_address",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "FRU Info Device Address    : %Xh %s\n",
                  (unsigned char) val,
                  (val ? "" : "(Unspecified)"));

  _FIID_OBJ_GET (cmd_rs,
                 "sdr_device_address",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "SDR Device Address         : %Xh\n",
                  (unsigned char) val);

  _FIID_OBJ_GET (cmd_rs,
                 "sel_device_address",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "SEL Device Address         : %Xh\n",
                  (unsigned char) val);

  _FIID_OBJ_GET (cmd_rs,
                 "system_management_device_address",
                 &val);
  pstdout_printf (state_data->pstate, 
                  "Sys Mgmt Device Address    : %Xh\n",
                  (unsigned char) val);

  if (fiid_obj_get (cmd_rs, "bridge_device_address", &val) >= 0) 
    {
      pstdout_printf (state_data->pstate, 
                      "Bridge Device Address      : %Xh\n",
                      (unsigned char) val);
    } 
  else 
    {
      pstdout_printf (state_data->pstate, 
                      "Bridge Device Address      : 20h (assuming default)\n");
    }

  rv = 0;
cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (cmd_rs);
  return rv;
}

int
run_cmd_args (ipmi_chassis_state_data_t *state_data)
{
  int rv = -1;

  assert (state_data);
  
  switch (state_data->prog_data->args->cmd) 
    {
      case IPMI_CMD_GET_CHASSIS_CAPABILITIES:
        get_chassis_capabilities (state_data);
        break;
    
      case IPMI_CMD_GET_CHASSIS_STATUS:
        get_chassis_status (state_data);
        break;
     
      case IPMI_CMD_CHASSIS_CONTROL:
        chassis_control (state_data);
        break;
    
      case IPMI_CMD_CHASSIS_IDENTIFY:
        chassis_identify (state_data);
        break;
    
      case IPMI_CMD_SET_POWER_RESTORE_POLICY: 
        set_power_restore_policy (state_data);
        break;
    
      case IPMI_CMD_GET_SYSTEM_RESTART_CAUSE:
        get_system_restart_cause (state_data);
        break;
    
      case IPMI_CMD_GET_POWER_ON_HOURS_COUNTER:
        get_power_on_hours_counter (state_data);
        break;
    
      case IPMI_CMD_SET_POWER_CYCLE_INTERVAL:
        set_power_cycle_interval (state_data);
        break;
    
      case IPMI_CMD_SET_SYSTEM_BOOT_OPTIONS:
        set_boot_flags (state_data);
        break;
    
      case IPMI_CMD_GET_SYSTEM_BOOT_OPTIONS :
        get_boot_flags (state_data);
        break;
    
      default:
        pstdout_fprintf(state_data->pstate,
                        stderr,
                        "Error: No commands given\n");
        goto cleanup;
    }

  rv = 0;
cleanup:
  return (rv);
}

static int
_ipmi_chassis (pstdout_state_t pstate,
               const char *hostname,
               void *arg)
{
  ipmi_chassis_state_data_t state_data;
  ipmi_chassis_prog_data_t *prog_data;
  ipmi_device_t dev = NULL;
  char errmsg[IPMI_DEVICE_OPEN_ERRMSGLEN];
  int exit_code = -1;

  prog_data = (ipmi_chassis_prog_data_t *)arg;

  if (!(dev = ipmi_device_open(prog_data->progname,
                               hostname,
                               &(prog_data->args->common),
                               prog_data->debug_flags,
                               errmsg,
                               IPMI_DEVICE_OPEN_ERRMSGLEN)))
    {
      pstdout_fprintf(pstate,
                      stderr,
                      "%s\n",
                      errmsg);
      exit_code = EXIT_FAILURE;
      goto cleanup;
    }

  memset(&state_data, '\0', sizeof(ipmi_chassis_state_data_t));
  state_data.dev = dev;
  state_data.prog_data = prog_data;
  state_data.pstate = pstate;

  if (run_cmd_args (&state_data) < 0)
    {
      exit_code = EXIT_FAILURE;
      goto cleanup;
    }
 
  exit_code = 0;
 cleanup:
  if (dev)
    ipmi_close_device (dev);
  return exit_code;
}

int 
main (int argc, char **argv)
{
  ipmi_chassis_prog_data_t prog_data;
  struct ipmi_chassis_arguments cmd_args;
  int exit_code;
  int rv;

  ipmi_disable_coredump();
  
  prog_data.progname = argv[0];
  ipmi_chassis_argp_parse (argc, argv, &cmd_args);

  if (ipmi_chassis_args_validate (&cmd_args) < 0)
    return (EXIT_FAILURE);

  prog_data.args = &cmd_args;

#ifndef NDEBUG
  if (prog_data.args->common.debug)
    prog_data.debug_flags = IPMI_FLAGS_DEBUG_DUMP;
  else
    prog_data.debug_flags = IPMI_FLAGS_DEFAULT;
#else  /* NDEBUG */
  prog_data.debug_flags = IPMI_FLAGS_DEFAULT;
#endif /* NDEBUG */
  
  if (pstdout_setup(&(prog_data.args->common.host),
                    prog_data.args->hostrange.buffer_hostrange_output,
                    prog_data.args->hostrange.consolidate_hostrange_output,
                    prog_data.args->hostrange.fanout,
                    prog_data.args->hostrange.eliminate) < 0)
    {
      exit_code = EXIT_FAILURE;
      goto cleanup;
    }
  
  if ((rv = pstdout_launch(prog_data.args->common.host,
                           _ipmi_chassis,
                           &prog_data)) < 0)
    {
      fprintf(stderr,
              "pstdout_launch: %s\n",
              pstdout_strerror(pstdout_errnum));
      exit_code = EXIT_FAILURE;
      goto cleanup;
    }
  
  exit_code = rv;
 cleanup:
  return (exit_code);
}
