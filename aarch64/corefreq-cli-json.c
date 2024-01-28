/*
 * CoreFreq (C) 2015-2024 CYRIL COURTIAT
 * Contributors: CyrIng
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreq-ui.h"
#include "corefreq-cli-rsc.h"
#include "corefreq-cli.h"
#include "corefreq-cli-json.h"
#include "corefreq-cli-extra.h"

double timespecFloat(struct timespec time)
{
	return (double) time.tv_sec + (time.tv_nsec / 1000000000.0);
}

void JsonSysInfo(RO(SHM_STRUCT) *RO(Shm))
{
	char hexStr[32];
	signed int i = 0, i2 = 0, i3 = 0;
	unsigned int cpu;
	struct json_state s = { .depth = 0, .nested_state = {0},
				.write = json_writer_stdout };
	json_start_object(&s);
	json_key(&s, "Registration");
	{
		json_start_object(&s);
		json_key(&s, "AutoClock");
		json_literal(&s, "%d", RO(Shm)->Registration.AutoClock);
		json_key(&s, "Experimental");
		json_literal(&s, "%d", RO(Shm)->Registration.Experimental);
		json_key(&s, "HotPlug");
		json_literal(&s, "%d", !(RO(Shm)->Registration.HotPlug < 0));
		json_key(&s, "PCI");
		json_literal(&s, "%d", RO(Shm)->Registration.PCI);
		json_key(&s, "Interrupt");
		{
			json_start_object(&s);
			json_key(&s, "NMI_LOCAL");
			json_literal(&s, "%u", BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_LOCAL));
			json_key(&s, "NMI_UNKNOWN");
			json_literal(&s, "%u", BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_UNKNOWN));
			json_key(&s, "NMI_SERR");
			json_literal(&s, "%u", BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_SERR));
			json_key(&s, "NMI_IO_CHECK");
			json_literal(&s, "%u", BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_IO_CHECK));
			json_end_object(&s);
		}
		json_key(&s, "IdleRoute");
		json_literal(&s, "%d", RO(Shm)->Registration.Driver.Route);
		json_key(&s, "CPUidle");
		json_literal(&s, "%hu", RO(Shm)->Registration.Driver.CPUidle);
		json_key(&s, "CPUfreq");
		json_literal(&s, "%hu", RO(Shm)->Registration.Driver.CPUfreq);
		json_key(&s, "Governor");
		json_literal(&s, "%hu", RO(Shm)->Registration.Driver.Governor);
		json_key(&s, "ClockSource");
		json_literal(&s, "%hu", RO(Shm)->Registration.Driver.CS);
		json_key(&s, "Scope");
		{
			json_start_object(&s);
			snprintf(hexStr, 32, "0x%x", RO(Shm)->Proc.thermalFormula);
			json_key(&s, "Thermal");
			json_string(&s, hexStr);
			snprintf(hexStr, 32, "0x%x", RO(Shm)->Proc.voltageFormula);
			json_key(&s, "Voltage");
			json_string(&s, hexStr);
			snprintf(hexStr, 32, "0x%x", RO(Shm)->Proc.powerFormula);
			json_key(&s, "Power");
			json_string(&s, hexStr);
			json_end_object(&s);
		}
		json_end_object(&s);
	}
    if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1))
    {
	json_key(&s, "SysGate");
	{
		json_start_object(&s);

		json_key(&s, "tickReset");
		json_literal(&s, "%u", RO(Shm)->SysGate.tickReset);
		json_key(&s, "tickStep");
		json_literal(&s, "%u", RO(Shm)->SysGate.tickStep);
		json_key(&s, "trackTask");
		json_literal(&s, "%d", RO(Shm)->SysGate.trackTask);
		json_key(&s, "sortByField");
		json_literal(&s, "%d", RO(Shm)->SysGate.sortByField);
		json_key(&s, "reverseOrder");
		json_literal(&s, "%d", RO(Shm)->SysGate.reverseOrder);
		json_key(&s, "taskCount");
		json_literal(&s, "%d", RO(Shm)->SysGate.taskCount);
		json_key(&s, "taskList");

		json_start_arr(&s);
		for (i = 0; i < RO(Shm)->SysGate.taskCount; i++) {
			json_start_object(&s);

			json_key(&s, "runtime");
			json_literal(&s, "%llu", RO(Shm)->SysGate.taskList[i].runtime);
			json_key(&s, "usertime");
			json_literal(&s, "%llu", RO(Shm)->SysGate.taskList[i].usertime);
			json_key(&s, "systime");
			json_literal(&s, "%llu", RO(Shm)->SysGate.taskList[i].systime);

			json_key(&s, "pid");
			json_literal(&s, "%d", RO(Shm)->SysGate.taskList[i].pid);
			json_key(&s, "tgid");
			json_literal(&s, "%d", RO(Shm)->SysGate.taskList[i].tgid);
			json_key(&s, "ppid");
			json_literal(&s, "%d", RO(Shm)->SysGate.taskList[i].ppid);

			json_key(&s, "state");
			json_literal(&s, "%hu", RO(Shm)->SysGate.taskList[i].state);
			json_key(&s, "wake_cpu");
			json_literal(&s, "%hu", RO(Shm)->SysGate.taskList[i].wake_cpu);
			json_key(&s, "comm");
			json_string(&s, RO(Shm)->SysGate.taskList[i].comm);

			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "memInfo");
		{
			json_start_object(&s);
			json_key(&s, "totalram");
			json_literal(&s, "%llu", RO(Shm)->SysGate.memInfo.totalram);
			json_key(&s, "sharedram");
			json_literal(&s, "%llu", RO(Shm)->SysGate.memInfo.sharedram);
			json_key(&s, "freeram");
			json_literal(&s, "%llu", RO(Shm)->SysGate.memInfo.freeram);
			json_key(&s, "bufferram");
			json_literal(&s, "%llu", RO(Shm)->SysGate.memInfo.bufferram);
			json_key(&s, "totalhigh");
			json_literal(&s, "%llu", RO(Shm)->SysGate.memInfo.totalhigh);
			json_key(&s, "freehigh");
			json_literal(&s, "%llu", RO(Shm)->SysGate.memInfo.freehigh);
			json_end_object(&s);
		}

		json_key(&s, "kernel");
		{
			json_start_object(&s);
			json_key(&s, "version");
			json_literal(&s, "%u", RO(Shm)->SysGate.kernel.version);
			json_key(&s, "major");
			json_literal(&s, "%u", RO(Shm)->SysGate.kernel.major);
			json_key(&s, "minor");
			json_literal(&s, "%u", RO(Shm)->SysGate.kernel.minor);
			json_end_object(&s);
		}
		json_key(&s, "sysname");
		json_string(&s, RO(Shm)->SysGate.sysname);
		json_key(&s, "release");
		json_string(&s, RO(Shm)->SysGate.release);
		json_key(&s, "version");
		json_string(&s, RO(Shm)->SysGate.version);
		json_key(&s, "machine");
		json_string(&s, RO(Shm)->SysGate.machine);

		json_key(&s, "SubDriver");
		{
			json_start_object(&s);

		    if (strlen(RO(Shm)->CS.array) > 0) {
			json_key(&s, "Clock_Source");
			json_string(&s, RO(Shm)->CS.array);
		    }
		    if (strlen(RO(Shm)->SysGate.OS.FreqDriver.Name) > 0) {
			json_key(&s, "CPU_Freq");
			json_string(&s, RO(Shm)->SysGate.OS.FreqDriver.Name);
		    }
		    if (strlen(RO(Shm)->SysGate.OS.FreqDriver.Governor) > 0) {
			json_key(&s, "Governor");
			json_string(&s, RO(Shm)->SysGate.OS.FreqDriver.Governor);
		    }
		    if (strlen(RO(Shm)->SysGate.OS.IdleDriver.Name) > 0) {
			json_key(&s, "CPU_Idle");
			json_string(&s, RO(Shm)->SysGate.OS.IdleDriver.Name);
		    }

			json_end_object(&s);
		}

		json_end_object(&s);
	}
    }
	json_key(&s, "Sleep");
	{
		json_start_object(&s);
		json_key(&s, "Interval");
		json_literal(&s, "%u", RO(Shm)->Sleep.Interval);
		json_key(&s, "pollingWait");
		json_literal(&s, "%f", timespecFloat(RO(Shm)->Sleep.pollingWait));
		json_key(&s, "ringWaiting");
		json_literal(&s, "%f", timespecFloat(RO(Shm)->Sleep.ringWaiting));
		json_key(&s, "childWaiting");
		json_literal(&s, "%f", timespecFloat(RO(Shm)->Sleep.childWaiting));
		json_key(&s, "sliceWaiting");
		json_literal(&s, "%f", timespecFloat(RO(Shm)->Sleep.sliceWaiting));
		json_end_object(&s);
	}

	json_key(&s, "ShmName");
	json_string(&s, RO(Shm)->ShmName);
	json_key(&s, "App");
	{
		json_start_object(&s);
		json_key(&s, "Svr");
		json_literal(&s, "%d", RO(Shm)->App.Svr);
		json_key(&s, "Cli");
		json_literal(&s, "%d", RO(Shm)->App.Cli);
		json_key(&s, "GUI");
		json_literal(&s, "%d", RO(Shm)->App.GUI);
		json_end_object(&s);
	}
	json_key(&s, "Uncore");
	{
		json_start_object(&s);
		json_key(&s, "Boost");
		{
			json_start_arr(&s);
			for (i = 0; i < UNCORE_RATIO_SIZE; i++) {
				json_literal(&s, "%u", RO(Shm)->Uncore.Boost[i]);
			}
			json_end_arr(&s);
		}
		json_key(&s, "Bus");
		{
			json_start_object(&s);
			json_key(&s, "Speed");
			json_literal(&s, "%u", RO(Shm)->Uncore.Bus.Speed);
			json_key(&s, "Rate");
			json_literal(&s, "%u", RO(Shm)->Uncore.Bus.Rate);
			json_end_object(&s);
		}

		json_key(&s, "MC");
		json_start_arr(&s);
		for (i = 0; i < MC_MAX_CTRL; i++) {
			{
				json_start_object(&s);
				json_key(&s, "Channel");
				json_start_arr(&s);
				{
					for (i2 = 0; i2 < MC_MAX_CHA; i2++) {
						json_start_object(&s);
						json_key(&s, "Timing");
						{
							json_start_object(&s);
							json_key(&s, "tCL");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tCL);
							json_key(&s, "tRCD_R");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRCD_RD);
							json_key(&s, "tRCD_W");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRCD_WR);
							json_key(&s, "tRP");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRP);
							json_key(&s, "tRAS");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRAS);
							json_key(&s, "tRRD");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRRD);
							json_key(&s, "tRFC");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRFC);
							json_key(&s, "tWR");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tWR);
							json_key(&s, "tRTPr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tRTPr);
							json_key(&s, "tWTPr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tWTPr);
							json_key(&s, "tFAW");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tFAW);
							json_key(&s, "B2B");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.B2B);
							json_key(&s, "tCWL");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tCWL);
							json_key(&s, "CMD_Rate");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.CMD_Rate);
							json_key(&s, "tsrRdTRd");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tsrRdTRd);
							json_key(&s, "tdrRdTRd");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tdrRdTRd);
							json_key(&s, "tddRdTRd");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tddRdTRd);
							json_key(&s, "tsrRdTWr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tsrRdTWr);
							json_key(&s, "tdrRdTWr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tdrRdTWr);
							json_key(&s, "tddRdTWr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tddRdTWr);
							json_key(&s, "tsrWrTRd");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tsrWrTRd);
							json_key(&s, "tdrWrTRd");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tdrWrTRd);
							json_key(&s, "tddWrTRd");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tddWrTRd);
							json_key(&s, "tsrWrTWr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tsrWrTWr);
							json_key(&s, "tdrWrTWr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tdrWrTWr);
							json_key(&s, "tddWrTWr");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.tddWrTWr);
							json_key(&s, "ECC");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].Timing.ECC);

							json_end_object(&s);
						}
						json_key(&s, "DIMM");
						json_start_arr(&s);
						for (i3 = 0; i3 < MC_MAX_DIMM; i3++) {
							json_start_object(&s);
							json_key(&s, "Size");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].DIMM[i3].Size);
							json_key(&s, "Rows");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].DIMM[i3].Rows);
							json_key(&s, "Cols");
							json_literal(&s, "%u", RO(Shm)->Uncore.MC[i].Channel[i2].DIMM[i3].Cols);
							json_key(&s, "Banks");
							json_literal(&s, "%hu", RO(Shm)->Uncore.MC[i].Channel[i2].DIMM[i3].Banks);
							json_key(&s, "Ranks");
							json_literal(&s, "%hu", RO(Shm)->Uncore.MC[i].Channel[i2].DIMM[i3].Ranks);
							json_end_object(&s);
						}
						json_end_arr(&s);
						json_end_object(&s);
					}
				}
				json_end_arr(&s);
				json_key(&s, "SlotCount");
				json_literal(&s, "%hu", RO(Shm)->Uncore.MC[i].SlotCount);
				json_key(&s, "ChannelCount");
				json_literal(&s, "%hu", RO(Shm)->Uncore.MC[i].ChannelCount);
				json_end_object(&s);
			}
		}
		json_end_arr(&s);

		json_key(&s, "CtrlSpeed");
		json_literal(&s, "%llu", RO(Shm)->Uncore.CtrlSpeed);
		json_key(&s, "CtrlCount");
		json_literal(&s, "%llu", RO(Shm)->Uncore.CtrlCount);
		json_key(&s, "Unit");
		{
			json_start_object(&s);
			json_key(&s, "Bus_Rate");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Uncore.Unit.Bus_Rate);
			json_key(&s, "BusSpeed");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Uncore.Unit.BusSpeed);
			json_key(&s, "DDR_Rate");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Uncore.Unit.DDR_Rate);
			json_key(&s, "DDRSpeed");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Uncore.Unit.DDRSpeed);
			json_key(&s, "DDR_Ver");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Uncore.Unit.DDR_Ver);
			json_key(&s, "DDR_Std");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Uncore.Unit.DDR_Std);
			json_end_object(&s);
		}
		json_end_object(&s);
	}
	json_key(&s, "Proc");
	{
		json_start_object(&s);

		json_key(&s, "Features");
		{
			json_start_object(&s);
			json_key(&s, "Info");
			{
				json_start_object(&s);
				json_key(&s, "Vendor");
				{
					json_start_object(&s);
					json_key(&s, "CRC");
					json_literal(&s, "%u", RO(Shm)->Proc.Features.Info.Vendor.CRC);
					json_key(&s, "ID");
					json_string(&s, RO(Shm)->Proc.Features.Info.Vendor.ID);
					json_end_object(&s);
				}
				json_key(&s, "Hypervisor");
				{
					json_start_object(&s);
					json_key(&s, "CRC");
					json_literal(&s, "%u", RO(Shm)->Proc.Features.Info.Hypervisor.CRC);
					json_key(&s, "ID");
					json_string(&s, RO(Shm)->Proc.Features.Info.Hypervisor.ID);
					json_end_object(&s);
				}

				json_end_object(&s);
			}
			json_key(&s, "Std");
			{
				json_start_object(&s);
				json_key(&s, "MIDR");
				{
				    json_start_object(&s);
				    json_key(&s, "Stepping");
				    {
					json_start_object(&s);
					snprintf(hexStr, 32, "0x%x", RO(Shm)->Proc.Features.Info.Signature.Stepping);
					json_key(&s, "Revision");
					json_string(&s, hexStr);
					json_end_object(&s);
				    }
					json_key(&s, "Model");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Info.Signature.Model);
					json_key(&s, "Family");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Info.Signature.Family);
					json_key(&s, "ExtModel");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Info.Signature.ExtModel);
					json_key(&s, "ExtFamily");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Info.Signature.ExtFamily);
					json_end_object(&s);
				}
				json_key(&s, "DFR1");
				{
					json_start_object(&s);
					json_key(&s, "EBEP");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.EBEP);
					json_end_object(&s);
				}
				json_key(&s, "ISAR0");
				{
					json_start_object(&s);
					json_key(&s, "AES");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.AES);
					json_key(&s, "SHA1");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SHA1);
					json_key(&s, "SHA256");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SHA256);
					json_key(&s, "SHA512");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SHA512);
					json_key(&s, "SHA3");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SHA3);
					json_key(&s, "CRC32");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.CRC32);
					json_key(&s, "CAS");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.CAS);
					json_key(&s, "DP");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.DP);
					json_key(&s, "SM3");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SM3);
					json_key(&s, "SM4");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SM4);
					json_key(&s, "RAND");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.RAND);
					json_key(&s, "TME");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.TME);
					json_key(&s, "FHM");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.FHM);
					json_key(&s, "TS");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.TS);
					json_key(&s, "TLB");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.TLB);
					json_key(&s, "RDMA");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.RDMA);
					json_end_object(&s);
				}
				json_key(&s, "ISAR1");
				{
					json_start_object(&s);
					json_key(&s, "FCMA");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.FCMA);
					json_key(&s, "LRCPC");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.LRCPC);
					json_end_object(&s);
				}
				json_key(&s, "MMFR1");
				{
					json_start_object(&s);
					json_key(&s, "PAN");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PAN);
					json_key(&s, "VHE");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.VHE);
					json_end_object(&s);
				}
				json_key(&s, "MMFR2");
				{
					json_start_object(&s);
					json_key(&s, "UAO");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.UAO);
					json_end_object(&s);
				}
				json_key(&s, "PFR0");
				{
					json_start_object(&s);
					json_key(&s, "FP");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.FP);
					json_key(&s, "SIMD");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SIMD);
					json_key(&s, "GIC");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.GIC);
					json_key(&s, "SVE");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SVE);
					json_key(&s, "DIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.DIT);
					json_end_object(&s);
				}
				json_key(&s, "PFR1");
				{
					json_start_object(&s);
					json_key(&s, "MTE");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MTE);
					json_key(&s, "NMI");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.NMI);
					json_key(&s, "SME");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.SME);
					json_end_object(&s);
				}
				json_key(&s, "MISC");
				{
					json_start_object(&s);
					json_key(&s, "HTT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.HTT);
					json_key(&s, "TSC");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.TSC);
					json_key(&s, "MONITOR");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MONITOR);
					json_key(&s, "Hybrid");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Hybrid);
					json_key(&s, "ACPI");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.ACPI);
					json_key(&s, "Hyperv");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Hyperv);
					json_end_object(&s);
				}
				json_end_object(&s);
			}
			json_key(&s, "MWait");
			{
				json_start_object(&s);
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "SubCstate_C0_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT0);
					json_key(&s, "SubCstate_C1_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT1);
					json_key(&s, "SubCstate_C2_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT2);
					json_key(&s, "SubCstate_C3_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT3);
					json_key(&s, "SubCstate_C4_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT4);
					json_key(&s, "SubCstate_C5_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT5);
					json_key(&s, "SubCstate_C6_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT6);
					json_key(&s, "SubCstate_C7_MWAIT");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.MWait.SubCstate_MWAIT7);

					json_end_object(&s);
				}
				json_end_object(&s);
			}
			json_key(&s, "Power");
			{
		json_start_object(&s);

		json_key(&s, "EAX");
		{
			json_start_object(&s);
			json_key(&s, "DTS");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Power.DTS);
			json_key(&s, "PLN");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Power.PLN);
			json_key(&s, "PTM");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Power.PTM);
			json_key(&s, "HWP_Registers");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Power.HWP_Reg);
			json_key(&s, "Turbo_V3");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Power.Turbo_V3);
			json_key(&s, "HCF_Cap");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Power.HCF_Cap);
			json_end_object(&s);
		}
		json_end_object(&s);
			}

			json_key(&s, "PerfMon");
			{
				json_start_object(&s);

				json_key(&s, "EAX");
				{
					json_start_object(&s);
					json_key(&s, "Version");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.Version);
					json_key(&s, "MonCtrs");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.MonCtrs);
					json_key(&s, "PMC_LLC");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Factory.PMC.LLC);
					json_key(&s, "PMC_NB");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.Factory.PMC.NB);
					json_key(&s, "MonWidth");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.MonWidth);
					json_key(&s, "VectorSz");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.VectorSz);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "CoreCycles");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.CoreCycles);
					json_key(&s, "InstrRetired");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.InstrRetired);
					json_key(&s, "RefCycles");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.RefCycles);
					json_key(&s, "LLC_Ref");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.LLC_Ref);
					json_key(&s, "LLC_Misses");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.LLC_Misses);
					json_key(&s, "BranchRetired");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.BranchRetired);
					json_key(&s, "BranchMispred");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.BranchMispred);
					json_key(&s, "TopdownSlots");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.TopdownSlots);
					json_key(&s, "ReservedBits");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.ReservedBits);
					json_end_object(&s);
				}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					json_key(&s, "FixCtrs_Mask");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.FixCtrs_Mask);
					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "FixCtrs");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.FixCtrs);
					json_key(&s, "FixWidth");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.FixWidth);
					json_key(&s, "AnyThread_Deprecation");
					json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Features.PerfMon.AnyThread_Dprec);
					json_end_object(&s);
				}

				json_end_object(&s);
			}
			json_key(&s, "FactoryFreq");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.Factory.Freq);

			json_key(&s, "InvariantTSC");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.InvariantTSC);
			json_key(&s, "HyperThreading");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.HyperThreading);
			json_key(&s, "HTT_Enable");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.HTT_Enable);
			json_key(&s, "Turbo_Unlock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.Turbo_Unlock);
			json_key(&s, "TDP_Unlock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.TDP_Unlock);
			json_key(&s, "TDP_Levels");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.TDP_Levels);
			json_key(&s, "TDP_Cfg_Lock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.TDP_Cfg_Lock);
			json_key(&s, "TDP_Cfg_Level");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.TDP_Cfg_Level);
			json_key(&s, "TurboActivation_Lock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.TurboActiv_Lock);
			json_key(&s, "TargetRatio_Unlock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.TgtRatio_Unlock);
			json_key(&s, "ClockRatio_Unlock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.ClkRatio_Unlock);
			json_key(&s, "Uncore_Unlock");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.Uncore_Unlock);
			json_key(&s, "ACPI_CPPC");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.ACPI_CPPC);
			json_key(&s, "HWP_Enable");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.HWP_Enable);
			json_key(&s, "Other_Capable");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.Other_Capable);
			json_key(&s, "SpecTurboRatio");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.SpecTurboRatio);
			json_key(&s, "SSBS");
			json_literal(&s, "%u", RO(Shm)->Proc.Features.SSBS);

			json_end_object(&s);
		}
		json_key(&s, "HypervisorID");
		json_literal(&s, "%llu", RO(Shm)->Proc.HypervisorID);
		json_key(&s, "Technology");
		{
			json_start_object(&s);
			json_key(&s, "Turbo");
			json_literal(&s, "%llu", RO(Shm)->Proc.Technology.Turbo);
			json_key(&s, "VM");
			json_literal(&s, "%llu", RO(Shm)->Proc.Technology.VM);
			json_key(&s, "IOMMU");
			json_literal(&s, "%llu", RO(Shm)->Proc.Technology.IOMMU);
			json_key(&s, "IOMMU_Ver_Major");
			json_literal(&s, "%llu", RO(Shm)->Proc.Technology.IOMMU_Ver_Major);
			json_key(&s, "IOMMU_Ver_Minor");
			json_literal(&s, "%llu", RO(Shm)->Proc.Technology.IOMMU_Ver_Minor);

			json_end_object(&s);
		}
		json_key(&s, "CPU");
		{
			json_start_object(&s);
			json_key(&s, "Count");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.CPU.Count);
			json_key(&s, "Online");
			json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.CPU.OnLine);
			json_end_object(&s);
		}
		json_key(&s, "Service");
		{
			json_start_object(&s);
			json_key(&s, "Proc");
			json_literal(&s, "%llu", RO(Shm)->Proc.Service.Proc);
			json_key(&s, "Service");
			{
				json_start_object(&s);
				json_key(&s, "Core");
				json_literal(&s, "%u", (unsigned) RO(Shm)->Proc.Service.Core);
				json_key(&s, "Thread");
				json_literal(&s, "%d", RO(Shm)->Proc.Service.Thread);
				json_key(&s, "Hybrid");
				json_literal(&s, "%d", RO(Shm)->Proc.Service.Hybrid);
				json_end_object(&s);
			}
			json_end_object(&s);
		}
		json_key(&s, "Boost");
		{
			json_start_arr(&s);
			for (i = 0; i < RATIO_SIZE; i++)
			{
				json_literal(&s, "%u", RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Boost[i]);
			}
			json_end_arr(&s);
		}

		json_key(&s, "PM_version");
		json_literal(&s, "%u", RO(Shm)->Proc.PM_version);

		json_key(&s, "Top");
		{
			json_start_object(&s);
			json_key(&s, "Relative");
			json_literal(&s, "%u", RO(Shm)->Proc.Top.Rel);
			json_key(&s, "Absolute");
			json_literal(&s, "%u", RO(Shm)->Proc.Top.Abs);
			json_end_object(&s);
		}

		json_key(&s, "Toggle");
		json_literal(&s, "%u", RO(Shm)->Proc.Toggle);

		json_key(&s, "FlipFlop");
		json_start_arr(&s);
		for (i = 0; i < 2; i++) {
			json_start_object(&s);
			json_key(&s, "Delta");
			{
				json_start_object(&s);

				json_key(&s, "PTSC");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PCLK);
				json_key(&s, "PC02");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC02);
				json_key(&s, "PC03");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC03);
				json_key(&s, "PC04");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC04);
				json_key(&s, "PC06");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC06);
				json_key(&s, "PC07");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC07);
				json_key(&s, "PC08");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC08);
				json_key(&s, "PC09");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC09);
				json_key(&s, "PC10");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.PC10);
				json_key(&s, "MC6");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.MC6);
				json_key(&s, "ACCU");
				json_start_arr(&s);
				for (i2 = 0; i2 < DOMAIN_SIZE; i2++) {
					json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Delta.ACCU[i2]);
				}
				json_end_arr(&s);

				json_end_object(&s);
			}

			json_key(&s, "Uncore");
			{
				json_start_object(&s);
				json_key(&s, "FC0");
				json_literal(&s, "%llu", RO(Shm)->Proc.FlipFlop[i].Uncore.FC0);
				json_end_object(&s);
			}

			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "State");
		{
			json_start_object(&s);
			json_key(&s, "PC02");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC02);
			json_key(&s, "PC03");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC03);
			json_key(&s, "PC04");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC04);
			json_key(&s, "PC06");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC06);
			json_key(&s, "PC07");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC07);
			json_key(&s, "PC08");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC08);
			json_key(&s, "PC09");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC09);
			json_key(&s, "PC10");
			json_literal(&s, "%f", RO(Shm)->Proc.State.PC10);
			json_key(&s, "MC6");
			json_literal(&s, "%f", RO(Shm)->Proc.State.MC6);
			json_key(&s, "Energy");
			json_start_arr(&s);
			for (i = 0; i < DOMAIN_SIZE; i++) {
				json_literal(&s, "%f", RO(Shm)->Proc.State.Energy[i].Current);
			}
			json_end_arr(&s);
			json_key(&s, "Power");
			json_start_arr(&s);
			for (i = 0; i < DOMAIN_SIZE; i++) {
				json_literal(&s, "%f", RO(Shm)->Proc.State.Power[i].Current);
			}
			json_end_arr(&s);
			json_end_object(&s);
		}
		json_key(&s, "Avg");
		{
			json_start_object(&s);
			json_key(&s, "Turbo");
			json_literal(&s, "%f", RO(Shm)->Proc.Avg.Turbo);
			json_key(&s, "C0");
			json_literal(&s, "%f", RO(Shm)->Proc.Avg.C0);
			json_key(&s, "C3");
			json_literal(&s, "%f", RO(Shm)->Proc.Avg.C3);
			json_key(&s, "C6");
			json_literal(&s, "%f", RO(Shm)->Proc.Avg.C6);
			json_key(&s, "C7");
			json_literal(&s, "%f", RO(Shm)->Proc.Avg.C7);
			json_key(&s, "C1");
			json_literal(&s, "%f", RO(Shm)->Proc.Avg.C1);

			json_end_object(&s);
		}
		json_key(&s, "Power");
		{
			json_start_object(&s);
			json_key(&s, "TDP");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.TDP);
			json_key(&s, "Min");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.Min);
			json_key(&s, "Max");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.Max);
		    {
			enum PWR_DOMAIN pw;
			json_key(&s, "PL1");
			{
				json_start_arr(&s);
				for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
				{
					json_literal(&s, "%u", RO(Shm)->Proc.Power.Domain[pw].PWL[PL1]);
				}
				json_end_arr(&s);
			}
			json_key(&s, "PL2");
			{
				json_start_arr(&s);
				for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
				{
					json_literal(&s, "%u", RO(Shm)->Proc.Power.Domain[pw].PWL[PL2]);
				}
				json_end_arr(&s);
			}
		    }
			json_key(&s, "EDC");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.EDC);
			json_key(&s, "TDC");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.TDC);
			json_key(&s, "Unit");
			{
				json_start_object(&s);
				json_key(&s, "Watts");
				json_literal(&s, "%f", RO(Shm)->Proc.Power.Unit.Watts);
				json_key(&s, "Joules");
				json_literal(&s, "%f", RO(Shm)->Proc.Power.Unit.Joules);
				json_key(&s, "Times");
				json_literal(&s, "%f", RO(Shm)->Proc.Power.Unit.Times);
				json_end_object(&s);
			}

			json_end_object(&s);
		}
		json_key(&s, "Current");
		{
			json_start_object(&s);
			json_key(&s, "EDC");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.EDC);
			json_key(&s, "TDC");
			json_literal(&s, "%u", RO(Shm)->Proc.Power.TDC);
			json_end_object(&s);
		}
		json_key(&s, "Brand");
		json_string(&s, RO(Shm)->Proc.Brand);

		json_key(&s, "Architecture");
		json_string(&s, RO(Shm)->Proc.Architecture);

		json_key(&s, "Mechanisms");
		{
			json_start_object(&s);
			json_key(&s, "CLRBHB");
			json_literal(&s, "%llu", RO(Shm)->Proc.Mechanisms.CLRBHB);
			json_key(&s, "CSV2");
			json_literal(&s, "%llu", RO(Shm)->Proc.Mechanisms.CSV2);
			json_key(&s, "CSV3");
			json_literal(&s, "%llu", RO(Shm)->Proc.Mechanisms.CSV3);
			json_key(&s, "SSBS");
			json_literal(&s, "%llu", RO(Shm)->Proc.Mechanisms.SSBS);
			json_end_object(&s);
		}
		json_end_object(&s);
	}

	json_key(&s, "Cpu");
	json_start_arr(&s);
	for (cpu = 0; (cpu < RO(Shm)->Proc.CPU.Count); cpu++)
	{
	struct FLIP_FLOP *CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

		json_start_object(&s);
		json_key(&s, "OffLine");
		json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].OffLine);
		json_key(&s, "Clock");
		{
			json_start_object(&s);
			json_key(&s, "Q");
			json_literal(&s, "%llu", CFlop->Clock.Q);
			json_key(&s, "R");
			json_literal(&s, "%llu", CFlop->Clock.R);
			json_key(&s, "Hz");
			json_literal(&s, "%llu", CFlop->Clock.Hz);
			json_end_object(&s);
		}
		json_key(&s, "Toggle");
		json_literal(&s, "%u", RO(Shm)->Cpu[cpu].Toggle);

		json_key(&s, "Boost");
		{
			json_start_arr(&s);
			for (i = 0; i < RATIO_SIZE; i++)
			{
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].Boost[i]);
			}
			json_end_arr(&s);
		}
		json_key(&s, "Topology");
		{
			json_start_object(&s);
			json_key(&s, "BSP");
			json_literal(&s, "%d", RO(Shm)->Cpu[cpu].Topology.BSP);
			json_key(&s, "MPID");
			json_literal(&s, "%d", RO(Shm)->Cpu[cpu].Topology.MPID);
			json_key(&s, "PackageID");
			json_literal(&s, "%d", RO(Shm)->Cpu[cpu].Topology.PackageID);
			json_key(&s, "Hybrid_ID");
			json_literal(&s, "%d", RO(Shm)->Cpu[cpu].Topology.Cluster.Hybrid_ID);
			json_key(&s, "CoreID");
			json_literal(&s, "%d", RO(Shm)->Cpu[cpu].Topology.CoreID);
			json_key(&s, "ThreadID");
			json_literal(&s, "%d", RO(Shm)->Cpu[cpu].Topology.ThreadID);
			json_key(&s, "Cache");
			json_start_arr(&s);
			for (i2 = 0; i2 < CACHE_MAX_LEVEL; i2++) {
				json_start_object(&s);
				json_key(&s, "Set");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].Topology.Cache[i2].Set);
				json_key(&s, "Size");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].Topology.Cache[i2].Size);
				json_key(&s, "LineSz");
				json_literal(&s, "%hu", RO(Shm)->Cpu[cpu].Topology.Cache[i2].LineSz);
				json_key(&s, "Way");
				json_literal(&s, "%hu", RO(Shm)->Cpu[cpu].Topology.Cache[i2].Way);
				json_key(&s, "Feature");
				{
					json_start_object(&s);
					json_key(&s, "WriteBack");
					json_literal(&s, "%hu", RO(Shm)->Cpu[cpu].Topology.Cache[i2].Feature.WriteBack);
					json_key(&s, "Inclusive");
					json_literal(&s, "%hu", RO(Shm)->Cpu[cpu].Topology.Cache[i2].Feature.Inclusive);
					json_end_object(&s);
				}
				json_end_object(&s);
			}

			json_end_arr(&s);
			json_end_object(&s);
		}
		json_key(&s, "PowerThermal");
		{
			json_start_object(&s);
			json_key(&s, "Limit");
			json_start_arr(&s);
			{
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST]);
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST]);
			}
			json_end_arr(&s);
			json_key(&s, "DutyCycle");
			{
				json_start_object(&s);
				json_key(&s, "ClockMod");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.DutyCycle.ClockMod);
				json_key(&s, "Extended");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.DutyCycle.Extended);
				json_end_object(&s);
			}
			json_key(&s, "PowerPolicy");
			json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.PowerPolicy);
			json_key(&s, "HWP");
			{
				json_start_object(&s);
				json_key(&s, "Capabilities");
				{
					json_start_object(&s);
					json_key(&s, "Lowest");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Lowest);
					json_key(&s, "Efficient");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Most_Efficient);
					json_key(&s, "Guaranteed");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Guaranteed);
					json_key(&s, "Highest");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Highest);
					json_end_object(&s);
				}
				json_key(&s, "Request");
				{
					json_start_object(&s);
					json_key(&s, "Minimum_Perf");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf);
					json_key(&s, "Maximum_Perf");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf);
					json_key(&s, "Desired_Perf");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf);
					json_key(&s, "Energy_Pref");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Energy_Pref);
					json_end_object(&s);
				}
				json_end_object(&s);
			}

			json_end_object(&s);
		}
		json_key(&s, "FlipFlop");
		json_start_arr(&s);
		for (i2 = 0; i2 < 2; i2++) {
			json_start_object(&s);
			json_key(&s, "Delta");
			{
				json_start_object(&s);
				json_key(&s, "INST");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.INST);
				json_key(&s, "C0");
				{
					json_start_object(&s);
					json_key(&s, "UCC");
					json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.C0.UCC);
					json_key(&s, "URC");
					json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.C0.URC);
					json_end_object(&s);
				}
				json_key(&s, "C3");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.C3);
				json_key(&s, "C6");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.C6);
				json_key(&s, "C7");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.C7);
				json_key(&s, "TSC");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.TSC);
				json_key(&s, "C1");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].FlipFlop[i2].Delta.C1);

				json_end_object(&s);
			}
			json_key(&s, "State");
			{
				json_start_object(&s);
				json_key(&s, "IPS");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.IPS);
				json_key(&s, "IPC");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.IPC);
				json_key(&s, "CPI");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.CPI);
				json_key(&s, "Turbo");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.Turbo);
				json_key(&s, "C0");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.C0);
				json_key(&s, "C3");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.C3);
				json_key(&s, "C6");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.C6);
				json_key(&s, "C7");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.C7);
				json_key(&s, "C1");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].State.C1);

				json_end_object(&s);
			}
			json_key(&s, "Relative");
			{
				json_start_object(&s);
				json_key(&s, "Ratio");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].Relative.Ratio);
				json_key(&s, "Freq");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].Relative.Freq);
				json_end_object(&s);
			}
			json_key(&s, "Absolute");
			{
				json_start_object(&s);
				json_key(&s, "Ratio");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].Absolute.Ratio.Perf);
				json_key(&s, "Freq");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].Absolute.Freq);
				json_end_object(&s);
			}
			json_key(&s, "Thermal");
			{
				json_start_object(&s);
				json_key(&s, "Sensor");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Thermal.Sensor);
				json_key(&s, "Temp");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Thermal.Temp);
				json_key(&s, "Events");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Thermal.Events);
				json_key(&s, "Target");
				json_start_arr(&s);
				{
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Thermal.Param.Offset[THERMAL_TARGET]);
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Thermal.Param.Offset[THERMAL_OFFSET_P1]);
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Thermal.Param.Offset[THERMAL_OFFSET_P2]);
				}
				json_end_arr(&s);
				json_end_object(&s);
			}
			json_key(&s, "Voltage");
			{
				json_start_object(&s);
				json_key(&s, "VID");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Voltage.VID);
				json_key(&s, "Vcore");
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].FlipFlop[i2].Voltage.Vcore);

				json_end_object(&s);
			}
			json_key(&s, "Counter");
			{
				json_start_object(&s);
				json_key(&s, "SMI");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Counter.SMI);
				json_key(&s, "NMI");
				{
					json_start_object(&s);
					json_key(&s, "LOCAL");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Counter.NMI.LOCAL);
					json_key(&s, "UNKNOWN");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Counter.NMI.UNKNOWN);
					json_key(&s, "PCISERR");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Counter.NMI.PCISERR);
					json_key(&s, "IOCHECK");
					json_literal(&s, "%u", RO(Shm)->Cpu[cpu].FlipFlop[i2].Counter.NMI.IOCHECK);
					json_end_object(&s);
				}
				json_end_object(&s);
			}
			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "Frequency");
		{
			json_start_object(&s);
			json_key(&s, "Relative");
			json_start_arr(&s);
			{
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].Relative.Freq[SENSOR_LOWEST]);
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].Relative.Freq[SENSOR_HIGHEST]);
			}
			json_end_arr(&s);
			json_key(&s, "Absolute");
			json_start_arr(&s);
			{
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].Absolute.Freq[SENSOR_LOWEST]);
				json_literal(&s, "%f", RO(Shm)->Cpu[cpu].Absolute.Freq[SENSOR_HIGHEST]);
			}
			json_end_arr(&s);
			json_end_object(&s);
		}

		json_key(&s, "SystemRegister");
		{
			json_start_object(&s);
			snprintf(hexStr, 32, "0x%llx", RO(Shm)->Cpu[cpu].SystemRegister.FLAGS);
			json_key(&s, "FLAGS");
			json_string(&s, hexStr);

			snprintf(hexStr, 32, "0x%llx", RO(Shm)->Cpu[cpu].SystemRegister.SCTLR);
			json_key(&s, "SCTLR");
			json_string(&s, hexStr);

			snprintf(hexStr, 32, "0x%llx", RO(Shm)->Cpu[cpu].SystemRegister.SCTLR2);
			json_key(&s, "SCTLR2");
			json_string(&s, hexStr);
			json_end_object(&s);
		}
		json_key(&s, "Slice");
		{
			json_start_object(&s);
			json_key(&s, "Delta");
			{
				json_start_object(&s);
				json_key(&s, "TSC");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].Slice.Delta.TSC);
				json_key(&s, "INST");
				json_literal(&s, "%llu", RO(Shm)->Cpu[cpu].Slice.Delta.INST);
				json_end_object(&s);
			}
			json_key(&s, "Counter");
			json_start_arr(&s);
			for (i3 = 0; i3 < 3; i3++) {
				json_start_object(&s);
				json_key(&s, "TSC");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].Slice.Counter[i3].TSC);
				json_key(&s, "INST");
				json_literal(&s, "%u", RO(Shm)->Cpu[cpu].Slice.Counter[i3].INST);
				json_end_object(&s);
			}
			json_end_arr(&s);
			json_end_object(&s);
		}

		json_end_object(&s);
	}
	json_end_arr(&s);

	json_end_object(&s);
}
