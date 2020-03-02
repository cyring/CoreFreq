/*
 * CoreFreq (C) 2015-2020 CYRIL INGENIERIE
 * Contributors: Andrew Gurinovich ; CyrIng
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreq-ui.h"
#include "corefreq-cli.h"
#include "corefreq-cli-json.h"
#include "corefreq-cli-extra.h"

double timespecFloat(struct timespec time)
{
	return ((double) time.tv_sec + (time.tv_nsec / 1000000000.0));
}

void JsonSysInfo(SHM_STRUCT *Shm, CELL_FUNC OutFunc)
{
	signed int i = 0, i2 = 0, i3 = 0;
	unsigned int cpu, vendor = Shm->Proc.Features.Info.Vendor.CRC;
	struct json_state s = {.depth = 0, .nested_state =
		{}, .write = json_writer_stdout};
	json_start_object(&s);
	json_key(&s, "Registration");
	{
		json_start_object(&s);
		json_key(&s, "Experimental");
		json_literal(&s, "%d", Shm->Registration.Experimental);
		json_key(&s, "HotPlug");
		json_literal(&s, "%d", Shm->Registration.HotPlug);
		json_key(&s, "PCI");
		json_literal(&s, "%d", Shm->Registration.PCI);
		json_key(&s, "Interrupt");
		{
			json_start_object(&s);
			json_key(&s, "NMI_LOCAL");
			json_literal(&s, "%u", BITVAL(Shm->Registration.NMI, BIT_NMI_LOCAL));
			json_key(&s, "NMI_UNKNOWN");
			json_literal(&s, "%u", BITVAL(Shm->Registration.NMI, BIT_NMI_UNKNOWN));
			json_key(&s, "NMI_SERR");
			json_literal(&s, "%u", BITVAL(Shm->Registration.NMI, BIT_NMI_SERR));
			json_key(&s, "NMI_IO_CHECK");
			json_literal(&s, "%u", BITVAL(Shm->Registration.NMI, BIT_NMI_IO_CHECK));
			json_end_object(&s);
		}
		json_key(&s, "CPUidle");
		json_literal(&s, "%hu", Shm->Registration.Driver.CPUidle);
		json_key(&s, "CPUfreq");
		json_literal(&s, "%hu", Shm->Registration.Driver.CPUfreq);
		json_end_object(&s);
	}
    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
    {
	json_key(&s, "SysGate");
	{
		json_start_object(&s);

		json_key(&s, "tickReset");
		json_literal(&s, "%u", Shm->SysGate.tickReset);
		json_key(&s, "tickStep");
		json_literal(&s, "%u", Shm->SysGate.tickStep);
		json_key(&s, "trackTask");
		json_literal(&s, "%d", Shm->SysGate.trackTask);
		json_key(&s, "sortByField");
		json_literal(&s, "%d", Shm->SysGate.sortByField);
		json_key(&s, "reverseOrder");
		json_literal(&s, "%d", Shm->SysGate.reverseOrder);
		json_key(&s, "taskCount");
		json_literal(&s, "%d", Shm->SysGate.taskCount);
		json_key(&s, "taskList");

		json_start_arr(&s);
		for (i = 0; i < Shm->SysGate.taskCount; i++) {
			json_start_object(&s);

			json_key(&s, "runtime");
			json_literal(&s, "%llu", Shm->SysGate.taskList[i].runtime);
			json_key(&s, "usertime");
			json_literal(&s, "%llu", Shm->SysGate.taskList[i].usertime);
			json_key(&s, "systime");
			json_literal(&s, "%llu", Shm->SysGate.taskList[i].systime);

			json_key(&s, "pid");
			json_literal(&s, "%d", Shm->SysGate.taskList[i].pid);
			json_key(&s, "tgid");
			json_literal(&s, "%d", Shm->SysGate.taskList[i].tgid);
			json_key(&s, "ppid");
			json_literal(&s, "%d", Shm->SysGate.taskList[i].ppid);

			json_key(&s, "state");
			json_literal(&s, "%hu", Shm->SysGate.taskList[i].state);
			json_key(&s, "wake_cpu");
			json_literal(&s, "%hu", Shm->SysGate.taskList[i].wake_cpu);
			json_key(&s, "comm");
			json_string(&s, Shm->SysGate.taskList[i].comm);

			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "memInfo");
		{
			json_start_object(&s);
			json_key(&s, "totalram");
			json_literal(&s, "%llu", Shm->SysGate.memInfo.totalram);
			json_key(&s, "sharedram");
			json_literal(&s, "%llu", Shm->SysGate.memInfo.sharedram);
			json_key(&s, "freeram");
			json_literal(&s, "%llu", Shm->SysGate.memInfo.freeram);
			json_key(&s, "bufferram");
			json_literal(&s, "%llu", Shm->SysGate.memInfo.bufferram);
			json_key(&s, "totalhigh");
			json_literal(&s, "%llu", Shm->SysGate.memInfo.totalhigh);
			json_key(&s, "freehigh");
			json_literal(&s, "%llu", Shm->SysGate.memInfo.freehigh);
			json_end_object(&s);
		}

		json_key(&s, "kernel");
		{
			json_start_object(&s);
			json_key(&s, "version");
			json_literal(&s, "%u", Shm->SysGate.kernel.version);
			json_key(&s, "major");
			json_literal(&s, "%u", Shm->SysGate.kernel.major);
			json_key(&s, "minor");
			json_literal(&s, "%u", Shm->SysGate.kernel.minor);
			json_end_object(&s);
		}
		json_key(&s, "sysname");
		json_string(&s, Shm->SysGate.sysname);
		json_key(&s, "release");
		json_string(&s, Shm->SysGate.release);
		json_key(&s, "version");
		json_string(&s, Shm->SysGate.version);
		json_key(&s, "machine");
		json_string(&s, Shm->SysGate.machine);

		json_key(&s, "SubDriver");
		{
			json_start_object(&s);

		    if (strlen(Shm->SysGate.OS.FreqDriver.Name) > 0) {
			json_key(&s, "CPU_Freq");
			json_string(&s, Shm->SysGate.OS.FreqDriver.Name);
		    }
		    if (strlen(Shm->SysGate.OS.FreqDriver.Governor) > 0) {
			json_key(&s, "Governor");
			json_string(&s, Shm->SysGate.OS.FreqDriver.Governor);
		    }
		    if (strlen(Shm->SysGate.OS.IdleDriver.Name) > 0) {
			json_key(&s, "CPU_Idle");
			json_string(&s, Shm->SysGate.OS.IdleDriver.Name);
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
		json_literal(&s, "%u", Shm->Sleep.Interval);
		json_key(&s, "pollingWait");
		json_literal(&s, "%f", timespecFloat(Shm->Sleep.pollingWait));
		json_key(&s, "ringWaiting");
		json_literal(&s, "%f", timespecFloat(Shm->Sleep.ringWaiting));
		json_key(&s, "childWaiting");
		json_literal(&s, "%f", timespecFloat(Shm->Sleep.childWaiting));
		json_key(&s, "sliceWaiting");
		json_literal(&s, "%f", timespecFloat(Shm->Sleep.sliceWaiting));
		json_end_object(&s);
	}

	json_key(&s, "ShmName");
	json_string(&s, Shm->ShmName);
	json_key(&s, "App");
	{
		json_start_object(&s);
		json_key(&s, "Svr");
		json_literal(&s, "%d", Shm->App.Svr);
		json_key(&s, "Cli");
		json_literal(&s, "%d", Shm->App.Cli);
		json_key(&s, "GUI");
		json_literal(&s, "%d", Shm->App.GUI);
		json_end_object(&s);
	}
	json_key(&s, "Uncore");
	{
		json_start_object(&s);
		json_key(&s, "Boost");
		{
			json_start_arr(&s);
			for (i = 0; i < UNCORE_RATIO_SIZE; i++) {
				json_literal(&s, "%u", Shm->Uncore.Boost[i]);
			}
			json_end_arr(&s);
		}
		json_key(&s, "Bus");
		{
			json_start_object(&s);
			json_key(&s, "Speed");
			json_literal(&s, "%u", Shm->Uncore.Bus.Speed);
			json_key(&s, "Rate");
			json_literal(&s, "%u", Shm->Uncore.Bus.Rate);
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
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tCL);
							json_key(&s, "tRCD");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tRCD);
							json_key(&s, "tRP");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tRP);
							json_key(&s, "tRAS");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tRAS);
							json_key(&s, "tRRD");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tRRD);
							json_key(&s, "tRFC");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tRFC);
							json_key(&s, "tWR");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tWR);
							json_key(&s, "tRTPr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tRTPr);
							json_key(&s, "tWTPr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tWTPr);
							json_key(&s, "tFAW");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tFAW);
							json_key(&s, "B2B");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.B2B);
							json_key(&s, "tCWL");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tCWL);
							json_key(&s, "CMD_Rate");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.CMD_Rate);
							json_key(&s, "tsrRdTRd");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tsrRdTRd);
							json_key(&s, "tdrRdTRd");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tdrRdTRd);
							json_key(&s, "tddRdTRd");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tddRdTRd);
							json_key(&s, "tsrRdTWr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tsrRdTWr);
							json_key(&s, "tdrRdTWr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tdrRdTWr);
							json_key(&s, "tddRdTWr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tddRdTWr);
							json_key(&s, "tsrWrTRd");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tsrWrTRd);
							json_key(&s, "tdrWrTRd");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tdrWrTRd);
							json_key(&s, "tddWrTRd");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tddWrTRd);
							json_key(&s, "tsrWrTWr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tsrWrTWr);
							json_key(&s, "tdrWrTWr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tdrWrTWr);
							json_key(&s, "tddWrTWr");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.tddWrTWr);
							json_key(&s, "ECC");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].Timing.ECC);

							json_end_object(&s);
						}
						json_key(&s, "DIMM");
						json_start_arr(&s);
						for (i3 = 0; i3 < MC_MAX_DIMM; i3++) {
							json_start_object(&s);
							json_key(&s, "Size");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].DIMM[i3].Size);
							json_key(&s, "Rows");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].DIMM[i3].Rows);
							json_key(&s, "Cols");
							json_literal(&s, "%u", Shm->Uncore.MC[i].Channel[i2].DIMM[i3].Cols);
							json_key(&s, "Banks");
							json_literal(&s, "%hu", Shm->Uncore.MC[i].Channel[i2].DIMM[i3].Banks);
							json_key(&s, "Ranks");
							json_literal(&s, "%hu", Shm->Uncore.MC[i].Channel[i2].DIMM[i3].Ranks);
							json_end_object(&s);
						}
						json_end_arr(&s);
						json_end_object(&s);
					}
				}
				json_end_arr(&s);
				json_key(&s, "SlotCount");
				json_literal(&s, "%hu", Shm->Uncore.MC[i].SlotCount);
				json_key(&s, "ChannelCount");
				json_literal(&s, "%hu", Shm->Uncore.MC[i].ChannelCount);
				json_end_object(&s);
			}
		}
		json_end_arr(&s);

		json_key(&s, "CtrlSpeed");
		json_literal(&s, "%llu", Shm->Uncore.CtrlSpeed);
		json_key(&s, "CtrlCount");
		json_literal(&s, "%llu", Shm->Uncore.CtrlCount);
		json_key(&s, "Unit");
		{
			json_start_object(&s);
			json_key(&s, "Bus_Rate");
			json_literal(&s, "%u", (unsigned) Shm->Uncore.Unit.Bus_Rate);
			json_key(&s, "BusSpeed");
			json_literal(&s, "%u", (unsigned) Shm->Uncore.Unit.BusSpeed);
			json_key(&s, "DDR_Rate");
			json_literal(&s, "%u", (unsigned) Shm->Uncore.Unit.DDR_Rate);
			json_key(&s, "DDRSpeed");
			json_literal(&s, "%u", (unsigned) Shm->Uncore.Unit.DDRSpeed);
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
				json_key(&s, "LargestStdFunc");
				json_literal(&s, "%u", Shm->Proc.Features.Info.LargestStdFunc);
				json_key(&s, "LargestExtFunc");
				json_literal(&s, "%u", Shm->Proc.Features.Info.LargestExtFunc);
				json_key(&s, "Vendor");
				{
					json_start_object(&s);
					json_key(&s, "CRC");
					json_literal(&s, "%u", Shm->Proc.Features.Info.Vendor.CRC);
					json_key(&s, "ID");
					json_string(&s, Shm->Proc.Features.Info.Vendor.ID);
					json_end_object(&s);
				}

				json_end_object(&s);
			}
			json_key(&s, "Std");
			{
				json_start_object(&s);
				json_key(&s, "EAX");
				{
					json_start_object(&s);
					json_key(&s, "Stepping");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.Stepping);
					json_key(&s, "Model");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.Model);
					json_key(&s, "Family");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.Family);
					json_key(&s, "ProcType");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.ProcType);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.Unused1);
					json_key(&s, "ExtModel");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.ExtModel);
					json_key(&s, "ExtFamily");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.ExtFamily);
					json_key(&s, "Unused2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EAX.Unused2);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "Brand_ID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EBX.Brand_ID);
					json_key(&s, "CLFSH_Size");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EBX.CLFSH_Size);
					json_key(&s, "Max_SMT_ID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EBX.Max_SMT_ID);
					json_key(&s, "Init_APIC_ID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EBX.Init_APIC_ID);
					json_end_object(&s);
				}
				json_key(&s, "EСX");
				{
					json_start_object(&s);
					json_key(&s, "SSE3");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.SSE3);
					json_key(&s, "PCLMULQDQ");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.PCLMULDQ);
					json_key(&s, "DTES64");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.DTES64);
					json_key(&s, "MONITOR");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.MONITOR);
					json_key(&s, "DS_CPL");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.DS_CPL);
					json_key(&s, "VMX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.VMX);
					json_key(&s, "SMX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.SMX);
					json_key(&s, "EIST");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.EIST);
					json_key(&s, "TM2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.TM2);
					json_key(&s, "SSSE3");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.SSSE3);
					json_key(&s, "CNXT_ID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.CNXT_ID);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.Unused1);
					json_key(&s, "FMA");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.FMA);
					json_key(&s, "CMPXCHG16B");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.CMPXCHG16);
					json_key(&s, "xTPR");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.xTPR);
					json_key(&s, "PDCM");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.PDCM);
					json_key(&s, "Unused2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.Unused2);
					json_key(&s, "PCID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.PCID);
					json_key(&s, "DCA");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.DCA);
					json_key(&s, "SSE41");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.SSE41);
					json_key(&s, "SSE42");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.SSE42);
					json_key(&s, "x2APIC");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.x2APIC);
					json_key(&s, "MOVBE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.MOVBE);
					json_key(&s, "POPCNT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.POPCNT);
					json_key(&s, "TSCDEAD");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.TSCDEAD);
					json_key(&s, "XSAVE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.XSAVE);
					json_key(&s, "OSXSAVE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.OSXSAVE);
					json_key(&s, "AVX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.AVX);
					json_key(&s, "F16C");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.F16C);
					json_key(&s, "RDRAND");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.RDRAND);
					json_key(&s, "Hyperv");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.ECX.Hyperv);
					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "FPU");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.FPU);
					json_key(&s, "VME");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.VME);
					json_key(&s, "DE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.DE);
					json_key(&s, "PSE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PSE);
					json_key(&s, "TSC");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.TSC);
					json_key(&s, "MSR");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.MSR);
					json_key(&s, "PAE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PAE);
					json_key(&s, "MCE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.MCE);
					json_key(&s, "CMPXCHG8B");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.CMPXCHG8);
					json_key(&s, "APIC");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.APIC);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.Unused1);
					json_key(&s, "SEP");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.SEP);
					json_key(&s, "MTRR");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.MTRR);
					json_key(&s, "PGE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PGE);
					json_key(&s, "MCA");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.MCA);
					json_key(&s, "CMOV");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.CMOV);
					json_key(&s, "PAT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PAT);
					json_key(&s, "PSE36");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PSE36);
					json_key(&s, "PSN");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PSN);
					json_key(&s, "CLFLUSH");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.CLFLUSH);
					json_key(&s, "Unused2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.Unused2);
					json_key(&s, "DS_PEBS");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.DS_PEBS);
					json_key(&s, "ACPI");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.ACPI);
					json_key(&s, "MMX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.MMX);
					json_key(&s, "FXSR");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.FXSR);
					json_key(&s, "SSE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.SSE);
					json_key(&s, "SSE2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.SSE2);
					json_key(&s, "SS");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.SS);
					json_key(&s, "HTT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.HTT);
					json_key(&s, "TM1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.TM1);
					json_key(&s, "Unused3");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.Unused3);
					json_key(&s, "PBE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Std.EDX.PBE);

					json_end_object(&s);
				}
				json_end_object(&s);
			}
			json_key(&s, "MWait");
			{
				json_start_object(&s);
				json_key(&s, "EAX");
				{
					json_start_object(&s);
					json_key(&s, "SmallestSize");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EAX.SmallestSize);
					json_key(&s, "ReservedBits");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EAX.ReservedBits);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "LargestSize");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EBX.LargestSize);
					json_key(&s, "ReservedBits");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EBX.ReservedBits);
					json_end_object(&s);
				}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					json_key(&s, "EMX_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.ECX.EMX_MWAIT);
					json_key(&s, "IBE_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.ECX.IBE_MWAIT);
					json_key(&s, "ReservedBits");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.ECX.ReservedBits);
					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "SubCstate_C0_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT0);
					json_key(&s, "SubCstate_C1_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT1);
					json_key(&s, "SubCstate_C2_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT2);
					json_key(&s, "SubCstate_C3_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT3);
					json_key(&s, "SubCstate_C4_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT4);
					json_key(&s, "SubCstate_C5_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT5);
					json_key(&s, "SubCstate_C6_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT6);
					json_key(&s, "SubCstate_C7_MWAIT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT7);

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
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.DTS);
			json_key(&s, "TurboIDA");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.TurboIDA);
			json_key(&s, "ARAT");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.ARAT);
			json_key(&s, "PLN");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.PLN);
			json_key(&s, "ECMD");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.ECMD);
			json_key(&s, "PTM");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.PTM);
			json_key(&s, "HWP_Registers");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Reg);
			json_key(&s, "HWP_Interrupt");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Int);
			json_key(&s, "HWP_Activity");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Act);
			json_key(&s, "HWP_EPP");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_EPP);
			json_key(&s, "HWP_Package");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Pkg);
			json_key(&s, "HDC_Registers");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HDC_Reg);
			json_key(&s, "Turbo_V3");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.Turbo_V3);
			json_key(&s, "HWP_Highest_Pref_Cap");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_HPrf);
			json_key(&s, "HWP_PECI");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_PECI);
			json_key(&s, "HWP_Flex");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Flex);
			json_key(&s, "HWP_Fast");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Fast);
			json_key(&s, "HWP_Idle_SMT");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EAX.HWP_Idle);

			json_end_object(&s);
		}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					if (vendor == CRC_INTEL) {
						json_key(&s, "HCF_Cap");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.HCF_Cap);
						json_key(&s, "ACNT_Cap");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.ACNT_Cap);
						json_key(&s, "Unused1");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.Unused1);
						json_key(&s, "SETBH");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.SETBH);
						json_key(&s, "Unused2");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.Unused2);
					} else if ((vendor == CRC_AMD) || (vendor == CRC_HYGON)) {
						json_key(&s, "EffFreq");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.EffFreq);
						json_key(&s, "NotUsed");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.ECX.NotUsed);
					} else {
						fprintf(stderr, "Unknown vendor");
					}

					json_end_object(&s);
				}
		json_key(&s, "EDX");
		{
			json_start_object(&s);
			json_key(&s, "Unused1");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.Power.EDX.Unused1);

			json_end_object(&s);
		}
		json_end_object(&s);
			}
			json_key(&s, "ExtFeature");
			{
				json_start_object(&s);

				json_key(&s, "EAX");
				{
					json_start_object(&s);
					json_key(&s, "MaxSubLeaf");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EAX.MaxSubLeaf);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "FSGSBASE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.FSGSBASE);
					json_key(&s, "TSC_ADJUST");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.TSC_ADJUST);
					json_key(&s, "SGX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.SGX);
					json_key(&s, "BMI1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.BMI1);
					json_key(&s, "HLE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.HLE);
					json_key(&s, "AVX2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX2);
					json_key(&s, "FDP_EXCPTN_x87");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.FDP_EXCPTN_x87);
					json_key(&s, "SMEP");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.SMEP);
					json_key(&s, "BMI2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.BMI2);
					json_key(&s, "FastStrings");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.FastStrings);
					json_key(&s, "INVPCID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.INVPCID);
					json_key(&s, "RTM");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.RTM);
					json_key(&s, "PQM");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.PQM);
					json_key(&s, "FPU_CS_DS");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.FPU_CS_DS);
					json_key(&s, "MPX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.MPX);
					json_key(&s, "PQE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.PQE);
					json_key(&s, "AVX_512F");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX_512F);
					json_key(&s, "AVX_512DQ");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX_512DQ);
					json_key(&s, "RDSEED");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.RDSEED);
					json_key(&s, "ADX");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.ADX);
					json_key(&s, "SMAP");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.SMAP);
					json_key(&s, "AVX512_IFMA");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX512_IFMA);
					json_key(&s, "CLFLUSHOPT");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.CLFLUSHOPT);
					json_key(&s, "CLWB");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.CLWB);
					json_key(&s, "ProcessorTrace");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.ProcessorTrace);
					json_key(&s, "AVX512PF");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX512PF);
					json_key(&s, "AVX512ER");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX512ER);
					json_key(&s, "AVX512CD");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX512CD);
					json_key(&s, "SHA");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.SHA);
					json_key(&s, "AVX512BW");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX512BW);
					json_key(&s, "AVX512VL");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EBX.AVX512VL);

					json_end_object(&s);
				}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					json_key(&s, "PREFETCHWT1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.PREFETCHWT1);
					json_key(&s, "AVX512_VBMI");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.AVX512_VBMI);
					json_key(&s, "UMIP");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.UMIP);
					json_key(&s, "PKU");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.PKU);
					json_key(&s, "OSPKE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.OSPKE);
					json_key(&s, "WAITPKG");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.WAITPKG);
					json_key(&s, "GFNI");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.GFNI);
					json_key(&s, "AVX512_VPOPCNTDQ");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.AVX512_VPOPCNTDQ);
					json_key(&s, "MAWAU");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.MAWAU);
					json_key(&s, "RDPID");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.RDPID);
					json_key(&s, "CLDEMOTE");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.CLDEMOTE);
					json_key(&s, "MOVDIRI");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.MOVDIRI);
					json_key(&s, "MOVDIR64B");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.MOVDIR64B);
					json_key(&s, "SGX_LC");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.ECX.SGX_LC);

					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "AVX512_4VNNIW");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.AVX512_4VNNIW);
					json_key(&s, "AVX512_4FMAPS");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.AVX512_4FMAPS);
					json_key(&s, "MD_CLEAR");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.MD_CLEAR_Cap);
					json_key(&s, "IBRS_IBPB");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.IBRS_IBPB_Cap);
					json_key(&s, "STIBP");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.STIBP_Cap);
					json_key(&s, "L1D_FLUSH");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.L1D_FLUSH_Cap);
					json_key(&s, "IA32_ARCH_CAPABILITIES");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP);
					json_key(&s, "IA32_CORE_CAPABILITIES");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.IA32_CORE_CAP);
					json_key(&s, "SSBD");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtFeature.EDX.SSBD_Cap);

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
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EAX.Version);
					json_key(&s, "MonCtrs");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EAX.MonCtrs);
					json_key(&s, "MonWidth");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EAX.MonWidth);
					json_key(&s, "VectorSz");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EAX.VectorSz);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "CoreCycles");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.CoreCycles);
					json_key(&s, "InstrRetired");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.InstrRetired);
					json_key(&s, "RefCycles");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.RefCycles);
					json_key(&s, "LLC_Ref");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.LLC_Ref);
					json_key(&s, "LLC_Misses");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.LLC_Misses);
					json_key(&s, "BranchRetired");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.BranchRetired);
					json_key(&s, "BranchMispred");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.BranchMispred);
					json_key(&s, "ReservedBits");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EBX.ReservedBits);
					json_end_object(&s);
				}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.ECX.Unused1);
					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "FixCtrs");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EDX.FixCtrs);
					json_key(&s, "FixWidth");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EDX.FixWidth);
					json_key(&s, "AnyThread_Deprecation");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.PerfMon.EDX.AnyThread_Dprec);
					json_end_object(&s);
				}

				json_end_object(&s);
			}
			json_key(&s, "ExtInfo");
			{
				json_start_object(&s);

				json_key(&s, "ECX");
				{
					json_start_object(&s);
					if (vendor == CRC_INTEL) {
						json_key(&s, "LahfSahf");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF);
					} else if ((vendor == CRC_AMD) || (vendor == CRC_HYGON)) {
						json_key(&s, "LahfSahf");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.LahfSahf);
						json_key(&s, "MP_Mode");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.MP_Mode);
						json_key(&s, "SVM");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.SVM);
						json_key(&s, "Ext_APIC");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.Ext_APIC);
						json_key(&s, "AltMov");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.AltMov);
						json_key(&s, "ABM");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.ABM);
						json_key(&s, "SSE4A");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.SSE4A);
						json_key(&s, "AlignSSE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.AlignSSE);
						json_key(&s, "PREFETCH");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.PREFETCH);
						json_key(&s, "OSVW");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.OSVW);
						json_key(&s, "IBS");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.IBS);
						json_key(&s, "XOP");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.XOP);
						json_key(&s, "SKINIT");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.SKINIT);
						json_key(&s, "WDT");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.WDT);
						json_key(&s, "NotUsed1");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.NotUsed1);
						json_key(&s, "LWP");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.LWP);
						json_key(&s, "FMA4");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.FMA4);
						json_key(&s, "TCE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.TCE);
						json_key(&s, "NotUsed2");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.NotUsed2);
						json_key(&s, "TBM");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.TBM);
						json_key(&s, "TopoExt");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.TopoExt);
						json_key(&s, "PerfCore");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.PerfCore);
						json_key(&s, "PerfNB");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.PerfNB);
						json_key(&s, "NotUsed3");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.NotUsed3);
						json_key(&s, "Data_BP");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.Data_BP);
						json_key(&s, "PerfTSC");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.PerfTSC);
						json_key(&s, "PerfL2I");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.PerfL2I);
						json_key(&s, "MWaitExt");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.MWaitExt);
						json_key(&s, "NotUsed4");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.ECX.NotUsed4);
					} else {
						fprintf(stderr, "Unknown vendor");
					}
					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					if (vendor == CRC_INTEL) {
						json_key(&s, "Unused1");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.Unused1);
						json_key(&s, "SYSCALL");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.SYSCALL);
						json_key(&s, "Unused2");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.Unused2);
						json_key(&s, "XD_Bit");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.XD_Bit);
						json_key(&s, "Unused3");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.Unused3);
						json_key(&s, "PG_1GB");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.PG_1GB);
						json_key(&s, "RdTSCP");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.RdTSCP);
						json_key(&s, "Unused4");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.Unused4);
						json_key(&s, "IA64");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.IA64);
						json_key(&s, "Unused5");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.Unused5);
					} else if ((vendor == CRC_AMD) || (vendor == CRC_HYGON)) {
						json_key(&s, "FPU");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.FPU);
						json_key(&s, "VME");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.VME);
						json_key(&s, "DE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.DE);
						json_key(&s, "PSE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.PSE);
						json_key(&s, "TSC");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.TSC);
						json_key(&s, "MSR");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.MSR);
						json_key(&s, "PAE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.PAE);
						json_key(&s, "MCE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.MCE);
						json_key(&s, "CMPXCH8");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.CMPXCH8);
						json_key(&s, "APIC");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.APIC);
						json_key(&s, "NotUsed1");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.NotUsed1);
						json_key(&s, "SEP");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.SEP);
						json_key(&s, "MTRR");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.MTRR);
						json_key(&s, "PGE");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.PGE);
						json_key(&s, "MCA");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.MCA);
						json_key(&s, "CMOV");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.CMOV);
						json_key(&s, "PAT");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.PAT);
						json_key(&s, "PSE36");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.PSE36);
						json_key(&s, "NotUsed2");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.NotUsed2);
						json_key(&s, "NX");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.NX);
						json_key(&s, "NotUsed3");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.NotUsed3);
						json_key(&s, "MMX_Ext");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.MMX_Ext);
						json_key(&s, "MMX");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.MMX);
						json_key(&s, "FXSR");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.FXSR);
						json_key(&s, "FFXSR");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.FFXSR);
						json_key(&s, "Page1GB");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.Page1GB);
						json_key(&s, "RDTSCP");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.RDTSCP);
						json_key(&s, "NotUsed4");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.NotUsed4);
						json_key(&s, "LM");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX.LM);
						json_key(&s, "_3DNowEx");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX._3DNowEx);
						json_key(&s, "_3DNow");
						json_literal(&s, "%u", (unsigned) Shm->Proc.Features.ExtInfo.EDX._3DNow);
					} else {
						fprintf(stderr, "Unknown vendor");
					}
					json_end_object(&s);
				}

				json_end_object(&s);
			}
			json_key(&s, "AdvPower");
			{
				json_start_object(&s);

				json_key(&s, "EAX");
				{
					json_start_object(&s);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EAX.Unused1);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EBX.Unused1);
					json_end_object(&s);
				}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					json_key(&s, "Unused1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.ECX.Unused1);
					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					if (vendor == CRC_INTEL) {
			json_key(&s, "Inv_TSC");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.Inv_TSC);
					} else if ((vendor == CRC_AMD) || (vendor == CRC_HYGON)) {
			json_key(&s, "TS");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.TS);
			json_key(&s, "FID");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.FID);
			json_key(&s, "VID");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.VID);
			json_key(&s, "TTP");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.TTP);
			json_key(&s, "TM");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.TM);
			json_key(&s, "STC");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.STC);
			json_key(&s, "_100MHz");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX._100MHz);
			json_key(&s, "HwPstate");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.HwPstate);
			json_key(&s, "TscInv");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.TscInv);
			json_key(&s, "CPB");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.CPB);
			json_key(&s, "EffFrqRO");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.EffFrqRO);
			json_key(&s, "ProcFeedback");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.ProcFb);
			json_key(&s, "ProcPower");
			json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.ProcPwr);
					} else {
						fprintf(stderr, "Unknown vendor");
					}
					json_end_object(&s);
				}

				json_end_object(&s);
			}
			json_key(&s, "leaf80000008");
			{
				json_start_object(&s);

				json_key(&s, "EAX");
				{
					json_start_object(&s);
					json_key(&s, "MaxPhysicalAddr");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EAX.MaxPhysicalAddr);
					json_key(&s, "MaxLinearAddr");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EAX.MaxLinearAddr);
					json_key(&s, "MaxGuestPhysAddr");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EAX.MaxGuestPhysAddr);
					json_key(&s, "Reserved");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EAX.Reserved);
					json_end_object(&s);
				}
				json_key(&s, "EBX");
				{
					json_start_object(&s);
					json_key(&s, "CLZERO");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EBX.CLZERO);
					json_key(&s, "IRPerf");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EBX.IRPerf);
					json_key(&s, "XSaveErPtr");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EBX.XSaveErPtr);
					json_key(&s, "Reserved");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.EBX.Reserved);
					json_end_object(&s);
				}
				json_key(&s, "ECX");
				{
					json_start_object(&s);
					json_key(&s, "NC");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.ECX.NC);
					json_key(&s, "Reserved1");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.ECX.Reserved1);
					json_key(&s, "ApicIdCoreIdSize");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.ECX.ApicIdCoreIdSize);
					json_key(&s, "PerfTscSize");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.ECX.PerfTscSize);
					json_key(&s, "Reserved2");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.leaf80000008.ECX.Reserved2);

					json_end_object(&s);
				}
				json_key(&s, "EDX");
				{
					json_start_object(&s);
					json_key(&s, "Reserved");
					json_literal(&s, "%u", (unsigned) Shm->Proc.Features.AdvPower.EDX.Reserved);
					json_end_object(&s);
				}
				json_end_object(&s);
			}
			json_key(&s, "FactoryFreq");
			json_literal(&s, "%u", Shm->Proc.Features.Factory.Freq);

			json_key(&s, "InvariantTSC");
			json_literal(&s, "%u", Shm->Proc.Features.InvariantTSC);
			json_key(&s, "HyperThreading");
			json_literal(&s, "%u", Shm->Proc.Features.HyperThreading);
			json_key(&s, "HTT_Enable");
			json_literal(&s, "%u", Shm->Proc.Features.HTT_Enable);
			json_key(&s, "Turbo_Unlock");
			json_literal(&s, "%u", Shm->Proc.Features.Turbo_Unlock);
			json_key(&s, "TDP_Unlock");
			json_literal(&s, "%u", Shm->Proc.Features.TDP_Unlock);
			json_key(&s, "TDP_Levels");
			json_literal(&s, "%u", Shm->Proc.Features.TDP_Levels);
			json_key(&s, "TDP_Cfg_Lock");
			json_literal(&s, "%u", Shm->Proc.Features.TDP_Cfg_Lock);
			json_key(&s, "TDP_Cfg_Level");
			json_literal(&s, "%u", Shm->Proc.Features.TDP_Cfg_Level);
			json_key(&s, "TurboActivation_Lock");
			json_literal(&s, "%u", Shm->Proc.Features.TurboActiv_Lock);
			json_key(&s, "TargetRatio_Unlock");
			json_literal(&s, "%u", Shm->Proc.Features.TgtRatio_Unlock);
			json_key(&s, "ClockRatio_Unlock");
			json_literal(&s, "%u", Shm->Proc.Features.ClkRatio_Unlock);
			json_key(&s, "SpecTurboRatio");
			json_literal(&s, "%u", Shm->Proc.Features.SpecTurboRatio);

			json_end_object(&s);
		}
		json_key(&s, "PowerNow");
		json_literal(&s, "%llu", Shm->Proc.PowerNow);
		json_key(&s, "Technology");
		{
			json_start_object(&s);
			json_key(&s, "PowerNow");
			json_literal(&s, "%llu", Shm->Proc.Technology.PowerNow);
			json_key(&s, "ODCM");
			json_literal(&s, "%llu", Shm->Proc.Technology.ODCM);
			json_key(&s, "PowerMgmt");
			json_literal(&s, "%llu", Shm->Proc.Technology.PowerMgmt);
			json_key(&s, "EIST");
			json_literal(&s, "%llu", Shm->Proc.Technology.EIST);
			json_key(&s, "Turbo");
			json_literal(&s, "%llu", Shm->Proc.Technology.Turbo);
			json_key(&s, "C1E");
			json_literal(&s, "%llu", Shm->Proc.Technology.C1E);
			json_key(&s, "C3A");
			json_literal(&s, "%llu", Shm->Proc.Technology.C3A);
			json_key(&s, "C1A");
			json_literal(&s, "%llu", Shm->Proc.Technology.C1A);
			json_key(&s, "C3U");
			json_literal(&s, "%llu", Shm->Proc.Technology.C3U);
			json_key(&s, "C1U");
			json_literal(&s, "%llu", Shm->Proc.Technology.C1U);
			json_key(&s, "CC6");
			json_literal(&s, "%llu", Shm->Proc.Technology.CC6);
			json_key(&s, "PC6");
			json_literal(&s, "%llu", Shm->Proc.Technology.PC6);
			json_key(&s, "SMM");
			json_literal(&s, "%llu", Shm->Proc.Technology.SMM);
			json_key(&s, "VM");
			json_literal(&s, "%llu", Shm->Proc.Technology.VM);
			json_key(&s, "IOMMU");
			json_literal(&s, "%llu", Shm->Proc.Technology.IOMMU);

			json_end_object(&s);
		}
		json_key(&s, "CPU");
		{
			json_start_object(&s);
			json_key(&s, "Count");
			json_literal(&s, "%u", (unsigned) Shm->Proc.CPU.Count);
			json_key(&s, "Online");
			json_literal(&s, "%u", (unsigned) Shm->Proc.CPU.OnLine);
			json_end_object(&s);
		}
		json_key(&s, "Service");
		{
			json_start_object(&s);
			json_key(&s, "Proc");
			json_literal(&s, "%llu", Shm->Proc.Service.Proc);
			json_key(&s, "Service");
			{
				json_start_object(&s);
				json_key(&s, "Core");
				json_literal(&s, "%u", (unsigned) Shm->Proc.Service.Core);
				json_key(&s, "Thread");
				json_literal(&s, "%d", Shm->Proc.Service.Thread);
				json_end_object(&s);
			}
			json_end_object(&s);
		}
		json_key(&s, "Boost");
		{
			json_start_arr(&s);
			for (i = 0; i < RATIO_SIZE; i++) {
				json_literal(&s, "%u", Shm->Proc.Boost[i]);
			}
			json_end_arr(&s);
		}

		json_key(&s, "PM_version");
		json_literal(&s, "%u", Shm->Proc.PM_version);

		json_key(&s, "Top");
		json_literal(&s, "%u", Shm->Proc.Top);
		json_key(&s, "Toggle");
		json_literal(&s, "%u", Shm->Proc.Toggle);

		json_key(&s, "FlipFlop");
		json_start_arr(&s);
		for (i = 0; i < 2; i++) {
			json_start_object(&s);
			json_key(&s, "Delta");
			{
				json_start_object(&s);

				json_key(&s, "PTSC");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PTSC);
				json_key(&s, "PC02");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC02);
				json_key(&s, "PC03");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC03);
				json_key(&s, "PC06");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC06);
				json_key(&s, "PC07");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC07);
				json_key(&s, "PC08");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC08);
				json_key(&s, "PC09");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC09);
				json_key(&s, "PC10");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.PC10);
				json_key(&s, "ACCU");
				json_start_arr(&s);
				for (i2 = 0; i2 < DOMAIN_SIZE; i2++) {
					json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Delta.ACCU[i2]);
				}
				json_end_arr(&s);

				json_end_object(&s);
			}

			json_key(&s, "Uncore");
			{
				json_start_object(&s);
				json_key(&s, "FC0");
				json_literal(&s, "%llu", Shm->Proc.FlipFlop[i].Uncore.FC0);
				json_end_object(&s);
			}

			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "State");
		{
			json_start_object(&s);
			json_key(&s, "PC02");
			json_literal(&s, "%f", Shm->Proc.State.PC02);
			json_key(&s, "PC03");
			json_literal(&s, "%f", Shm->Proc.State.PC03);
			json_key(&s, "PC06");
			json_literal(&s, "%f", Shm->Proc.State.PC06);
			json_key(&s, "PC07");
			json_literal(&s, "%f", Shm->Proc.State.PC07);
			json_key(&s, "PC08");
			json_literal(&s, "%f", Shm->Proc.State.PC08);
			json_key(&s, "PC09");
			json_literal(&s, "%f", Shm->Proc.State.PC09);
			json_key(&s, "PC10");
			json_literal(&s, "%f", Shm->Proc.State.PC10);
			json_key(&s, "Energy");
			json_start_arr(&s);
			for (i = 0; i < DOMAIN_SIZE; i++) {
				json_literal(&s, "%f", Shm->Proc.State.Energy[i]);
			}
			json_end_arr(&s);
			json_key(&s, "Power");
			json_start_arr(&s);
			for (i = 0; i < DOMAIN_SIZE; i++) {
				json_literal(&s, "%f", Shm->Proc.State.Power[i]);
			}
			json_end_arr(&s);
			json_end_object(&s);
		}
		json_key(&s, "Avg");
		{
			json_start_object(&s);
			json_key(&s, "Turbo");
			json_literal(&s, "%f", Shm->Proc.Avg.Turbo);
			json_key(&s, "C0");
			json_literal(&s, "%f", Shm->Proc.Avg.C0);
			json_key(&s, "C3");
			json_literal(&s, "%f", Shm->Proc.Avg.C3);
			json_key(&s, "C6");
			json_literal(&s, "%f", Shm->Proc.Avg.C6);
			json_key(&s, "C7");
			json_literal(&s, "%f", Shm->Proc.Avg.C7);
			json_key(&s, "C1");
			json_literal(&s, "%f", Shm->Proc.Avg.C1);

			json_end_object(&s);
		}
		json_key(&s, "Power");
		{
			json_start_object(&s);
			json_key(&s, "TDP");
			json_literal(&s, "%u", Shm->Proc.Power.TDP);
			json_key(&s, "Min");
			json_literal(&s, "%u", Shm->Proc.Power.Min);
			json_key(&s, "Max");
			json_literal(&s, "%u", Shm->Proc.Power.Max);
			json_key(&s, "Unit");
			{
				json_start_object(&s);
				json_key(&s, "Watts");
				json_literal(&s, "%f", Shm->Proc.Power.Unit.Watts);
				json_key(&s, "Joules");
				json_literal(&s, "%f", Shm->Proc.Power.Unit.Joules);
				json_key(&s, "Times");
				json_literal(&s, "%f", Shm->Proc.Power.Unit.Times);
				json_end_object(&s);
			}

			json_end_object(&s);
		}

		json_key(&s, "Brand");
		json_string(&s, Shm->Proc.Brand);

		json_key(&s, "Architecture");
		json_string(&s, Shm->Proc.Architecture);

		json_key(&s, "Mechanisms");
		{
			json_start_object(&s);
			json_key(&s, "IBRS");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.IBRS);
			json_key(&s, "STIBP");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.STIBP);
			json_key(&s, "SSBD");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.SSBD);
			json_key(&s, "L1DFL_VMENTRY_NO");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.L1DFL_VMENTRY_NO);
			json_key(&s, "RDCL_NO");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.RDCL_NO);
			json_key(&s, "IBRS_ALL");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.IBRS_ALL);
			json_key(&s, "RSBA");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.RSBA);
			json_key(&s, "SSB_NO");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.SSB_NO);
			json_key(&s, "MDS_NO");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.MDS_NO);
			json_key(&s, "PSCHANGE_MC_NO");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.PSCHANGE_MC_NO);
			json_key(&s, "TAA_NO");
			json_literal(&s, "%llu", Shm->Proc.Mechanisms.TAA_NO);

			json_end_object(&s);
		}
		json_end_object(&s);
	}

	json_key(&s, "Cpu");
	json_start_arr(&s);
	for (cpu = 0; (cpu < Shm->Proc.CPU.Count); cpu++)
	{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		json_start_object(&s);
		json_key(&s, "OffLine");
		json_literal(&s, "%llu", Shm->Cpu[cpu].OffLine);
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
		json_literal(&s, "%u", Shm->Cpu[cpu].Toggle);
		/* TODO: ... Query */
		json_key(&s, "Topology");
		{
			json_start_object(&s);
			json_key(&s, "ApicID");
			json_literal(&s, "%d", Shm->Cpu[cpu].Topology.ApicID);
		    if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		    || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
		    {
			json_key(&s, "CCX");
			json_literal(&s, "%d", Shm->Cpu[cpu].Topology.MP.CCX);
		    }
			json_key(&s, "CoreID");
			json_literal(&s, "%d", Shm->Cpu[cpu].Topology.CoreID);
			json_key(&s, "ThreadID");
			json_literal(&s, "%d", Shm->Cpu[cpu].Topology.ThreadID);
			json_key(&s, "PackageID");
			json_literal(&s, "%d", Shm->Cpu[cpu].Topology.PackageID);
			json_key(&s, "MP");
			{
				json_start_object(&s);
				json_key(&s, "BSP");
				json_literal(&s, "%d", Shm->Cpu[cpu].Topology.MP.BSP);
				json_key(&s, "x2APIC");
				json_literal(&s, "%d", Shm->Cpu[cpu].Topology.MP.x2APIC);
				json_end_object(&s);
			}
			json_key(&s, "Cache");
			json_start_arr(&s);
			for (i2 = 0; i2 < CACHE_MAX_LEVEL; i2++) {
				json_start_object(&s);
				json_key(&s, "Set");
				json_literal(&s, "%u", Shm->Cpu[cpu].Topology.Cache[i2].Set);
				json_key(&s, "Size");
				json_literal(&s, "%u", Shm->Cpu[cpu].Topology.Cache[i2].Size);
				json_key(&s, "LineSz");
				json_literal(&s, "%hu", Shm->Cpu[cpu].Topology.Cache[i2].LineSz);
				json_key(&s, "Part");
				json_literal(&s, "%hu", Shm->Cpu[cpu].Topology.Cache[i2].Part);
				json_key(&s, "Way");
				json_literal(&s, "%hu", Shm->Cpu[cpu].Topology.Cache[i2].Way);
				json_key(&s, "Feature");
				{
					json_start_object(&s);
					json_key(&s, "WriteBack");
					json_literal(&s, "%hu", Shm->Cpu[cpu].Topology.Cache[i2].Feature.WriteBack);
					json_key(&s, "Inclusive");
					json_literal(&s, "%hu", Shm->Cpu[cpu].Topology.Cache[i2].Feature.Inclusive);
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
			json_key(&s, "TM1");
			json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.TM1);
			json_key(&s, "TM2");
			json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.TM2);
			json_key(&s, "Limit");
			json_start_arr(&s);
			{
				json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST]);
				json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST]);
			}
			json_end_arr(&s);
			json_key(&s, "DutyCycle");
			{
				json_start_object(&s);
				json_key(&s, "ClockMod");
				json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.DutyCycle.ClockMod);
				json_key(&s, "Extended");
				json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.DutyCycle.Extended);
				json_end_object(&s);
			}
			json_key(&s, "PowerPolicy");
			json_literal(&s, "%u", Shm->Cpu[cpu].PowerThermal.PowerPolicy);
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
				json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.INST);
				json_key(&s, "C0");
				{
					json_start_object(&s);
					json_key(&s, "UCC");
					json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.C0.UCC);
					json_key(&s, "URC");
					json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.C0.URC);
					json_end_object(&s);
				}
				json_key(&s, "C3");
				json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.C3);
				json_key(&s, "C6");
				json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.C6);
				json_key(&s, "C7");
				json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.C7);
				json_key(&s, "TSC");
				json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.TSC);
				json_key(&s, "C1");
				json_literal(&s, "%llu", Shm->Cpu[cpu].FlipFlop[i2].Delta.C1);
				json_end_object(&s);
			}
			json_key(&s, "State");
			{
				json_start_object(&s);
				json_key(&s, "IPS");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.IPS);
				json_key(&s, "IPC");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.IPC);
				json_key(&s, "CPI");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.CPI);
				json_key(&s, "Turbo");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.Turbo);
				json_key(&s, "C0");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.C0);
				json_key(&s, "C3");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.C3);
				json_key(&s, "C6");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.C6);
				json_key(&s, "C7");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.C7);
				json_key(&s, "C1");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].State.C1);
				json_end_object(&s);
			}
			json_key(&s, "Relative");
			{
				json_start_object(&s);
				json_key(&s, "Ratio");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].Relative.Ratio);
				json_key(&s, "Freq");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].Relative.Freq);
				json_end_object(&s);
			}
			json_key(&s, "Thermal");
			{
				json_start_object(&s);
				json_key(&s, "Sensor");
				json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Thermal.Sensor);
				json_key(&s, "Temp");
				json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Thermal.Temp);
				json_key(&s, "Events");
				json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Thermal.Events);
				json_key(&s, "Target");
				json_start_arr(&s);
				{
					json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Thermal.Param.Offset[0]);
					json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Thermal.Param.Offset[1]);
				}
				json_end_arr(&s);
				json_end_object(&s);
			}
			json_key(&s, "Voltage");
			{
				json_start_object(&s);
				json_key(&s, "VID");
				json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Voltage.VID);
				json_key(&s, "Vcore");
				json_literal(&s, "%f", Shm->Cpu[cpu].FlipFlop[i2].Voltage.Vcore);

				json_end_object(&s);
			}
			json_key(&s, "Counter");
			{
				json_start_object(&s);
				json_key(&s, "SMI");
				json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Counter.SMI);
				json_key(&s, "NMI");
				{
					json_start_object(&s);
					json_key(&s, "LOCAL");
					json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Counter.NMI.LOCAL);
					json_key(&s, "UNKNOWN");
					json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Counter.NMI.UNKNOWN);
					json_key(&s, "PCISERR");
					json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Counter.NMI.PCISERR);
					json_key(&s, "IOCHECK");
					json_literal(&s, "%u", Shm->Cpu[cpu].FlipFlop[i2].Counter.NMI.IOCHECK);
					json_end_object(&s);
				}
				json_end_object(&s);
			}
			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "SystemRegister");
		{
			json_start_object(&s);
			json_key(&s, "RFLAGS");
			json_literal(&s, "%llu", Shm->Cpu[cpu].SystemRegister.RFLAGS);
			json_key(&s, "CR0");
			json_literal(&s, "%llu", Shm->Cpu[cpu].SystemRegister.CR0);
			json_key(&s, "CR3");
			json_literal(&s, "%llu", Shm->Cpu[cpu].SystemRegister.CR3);
			json_key(&s, "CR4");
			json_literal(&s, "%llu", Shm->Cpu[cpu].SystemRegister.CR4);
			json_key(&s, "EFCR");
			json_literal(&s, "%llu", Shm->Cpu[cpu].SystemRegister.EFCR);
			json_key(&s, "EFER");
			json_literal(&s, "%llu", Shm->Cpu[cpu].SystemRegister.EFER);
			json_end_object(&s);
		}
		json_key(&s, "CpuID");
		json_start_arr(&s);
		for (i2 = 0; i2 < CPUID_MAX_FUNC; i2++) {
			json_start_object(&s);
			json_key(&s, "func");
			json_literal(&s, "%u", Shm->Cpu[cpu].CpuID[i2].func);
			json_key(&s, "sub");
			json_literal(&s, "%u", Shm->Cpu[cpu].CpuID[i2].sub);
			json_key(&s, "reg");
			json_start_arr(&s);
			for (i3 = 0; i3 < 4; i3++) {
				json_literal(&s, "%u", Shm->Cpu[cpu].CpuID[i2].reg[i3]);
			}
			json_end_arr(&s);
			json_end_object(&s);
		}
		json_end_arr(&s);

		json_key(&s, "Slice");
		{
			json_start_object(&s);
			json_key(&s, "Delta");
			{
				json_start_object(&s);
				json_key(&s, "TSC");
				json_literal(&s, "%llu", Shm->Cpu[cpu].Slice.Delta.TSC);
				json_key(&s, "INST");
				json_literal(&s, "%llu", Shm->Cpu[cpu].Slice.Delta.INST);
				json_end_object(&s);
			}
			json_key(&s, "Counter");
			json_start_arr(&s);
			for (i3 = 0; i3 < 3; i3++) {
				json_start_object(&s);
				json_key(&s, "TSC");
				json_literal(&s, "%u", Shm->Cpu[cpu].Slice.Counter[i3].TSC);
				json_key(&s, "INST");
				json_literal(&s, "%u", Shm->Cpu[cpu].Slice.Counter[i3].INST);
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

