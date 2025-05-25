// backend/arm32/SimpleRegisterAllocator.cpp
#include <algorithm>
#include <vector>
#include "SimpleRegisterAllocator.h"
#include "Common.h"     // For minic_log
#include "Value.h"             // For Value, getIRName etc.
#include "PlatformArm32.h"     // For maxUsableRegNum
#include <typeinfo>           // For typeid

SimpleRegisterAllocator::SimpleRegisterAllocator() {}

// Helper function to log Value details
static std::string getValueDetails(Value* var) {
    if (!var) return "null_Value";
    return "'" + var->getIRName() + "' (Name: '" + var->getName() + "', DynType: " + typeid(*var).name() + 
           ", Ptr: " + std::to_string(reinterpret_cast<uintptr_t>(var)) + 
           ", LoadRegId: " + std::to_string(var->getLoadRegId()) + ")";
}


int SimpleRegisterAllocator::Allocate(Value * var, int32_t no_requested) {

	// ---- 修复：确保 bitmap_str 在这里声明和构建 ----
	std::string bitmap_str; 
	for (int i = 0; i < PlatformArm32::maxUsableRegNum; ++i) {
		 bitmap_str += (regBitmap.test(i) ? '1' : '0');
	 }
	// ---- 结束修复 ----
    std::string var_details = getValueDetails(var);
	for (int i = 0; i < PlatformArm32::maxUsableRegNum; ++i) {
		bitmap_str += (regBitmap.test(i) ? '1' : '0');
	}
    minic_log(LOG_DEBUG, "Allocator::Allocate called for Var: %s, Requested Reg: %d", 
              var_details.c_str(), no_requested);

    if (var) {
        int32_t current_load_reg_id_for_var = var->getLoadRegId();
        if (current_load_reg_id_for_var != -1) {
            // Value already has an associated register
            if (current_load_reg_id_for_var >= 0 && current_load_reg_id_for_var < PlatformArm32::maxUsableRegNum) {
                if (!regBitmap.test(current_load_reg_id_for_var)) {
                    minic_log(LOG_WARNING, "Allocator: Var %s claims reg %d but bitmap says free. Fixing bitmap.",
                              var_details.c_str(), current_load_reg_id_for_var);
                    bitmapSet(current_load_reg_id_for_var); 
                }
                // Move var to the end of regValues to mark it as recently used (for FIFO spill)
                auto it = std::find(regValues.begin(), regValues.end(), var);
                if (it != regValues.end()) {
                    regValues.erase(it);
                }
                regValues.push_back(var);
                minic_log(LOG_DEBUG, "Allocator: Var %s re-confirmed for already allocated reg %d.", 
                          var_details.c_str(), current_load_reg_id_for_var);
                return current_load_reg_id_for_var;
            } else {
                 minic_log(LOG_WARNING, "Allocator: Var %s has invalid cached loadRegId %d. Will re-allocate.",
                           var_details.c_str(), current_load_reg_id_for_var);
                 var->setLoadRegId(-1); // Clear invalid ID
            }
        }
    }

    int32_t regno = -1;

    // 1. Try requested register if specified and valid
    if ((no_requested != -1) && (no_requested >= 0 && no_requested < PlatformArm32::maxUsableRegNum)) {
        if (!regBitmap.test(no_requested)) {
            regno = no_requested;
            minic_log(LOG_DEBUG, "Allocator: Allocated requested reg %d for %s.", regno, var_details.c_str());
        } else {
            minic_log(LOG_DEBUG, "Allocator: Requested reg %d for %s is busy. Searching for free reg.", no_requested, var_details.c_str());
        }
    }

    // 2. If not allocated yet, search for any free register
    if (regno == -1) {
        for (int k = 0; k < PlatformArm32::maxUsableRegNum; ++k) {
            if (!regBitmap.test(k)) {
                regno = k;
                minic_log(LOG_DEBUG, "Allocator: Found free reg %d for %s.", regno, var_details.c_str());
                break;
            }
        }
    }

    // 3. If still not allocated, spill
    if (regno != -1) {
        bitmapSet(regno); // Mark as occupied
    } else {
		minic_log(LOG_DEBUG, "Allocator: No free regs for %s. Attempting spill. RegBitmap: %s", 
			getValueDetails(var).c_str(), bitmap_str.c_str());
        // No free registers, need to spill
        if (regValues.empty()) {
            minic_log(LOG_ERROR, "Allocator: No free registers and no values in regValues to spill! Cannot allocate for %s.",
                      var_details.c_str());
            return -1; 
        }
		minic_log(LOG_DEBUG, "Allocator: regValues before spill (size %zu):", regValues.size());
		for (size_t i = 0; i < regValues.size(); ++i) {
			minic_log(LOG_DEBUG, "  regValues[%zu]: %s", i, getValueDetails(regValues[i]).c_str());
		}
        Value * varToSpill = regValues.front(); // FIFO: oldest allocated
        int32_t spilledRegId = varToSpill->getLoadRegId();
        std::string spilled_var_details = getValueDetails(varToSpill);

        minic_log(LOG_DEBUG, "Allocator: Spilling Var %s from reg %d to allocate for Var %s.",
                  spilled_var_details.c_str(), spilledRegId, var_details.c_str());

        if (spilledRegId == -1 || spilledRegId >= PlatformArm32::maxUsableRegNum) {
            minic_log(LOG_ERROR, "Allocator: CRITICAL - VarToSpill %s has invalid stored regId %d! State inconsistent. Removing from list and failing allocation for %s.",
                      spilled_var_details.c_str(), spilledRegId, var_details.c_str());
            regValues.erase(regValues.begin()); // Remove problematic entry
            varToSpill->setLoadRegId(-1);      // Ensure it's disassociated
            return -1; // Fail current allocation
        }
        
        regno = spilledRegId; // This register is now free for 'var'

        // TODO: Generate actual spill code (STR instruction) for varToSpill if it's dirty and has a memory location.
        // This requires coordination with ILocArm32 or similar.
        // For now, we just disassociate it from the register.
        minic_log(LOG_INFO, "Allocator: (Simulated Spill) Value %s (originally in reg %d) needs to be stored to memory.", 
                  spilled_var_details.c_str(), spilledRegId);


        varToSpill->setLoadRegId(-1);      
        regValues.erase(regValues.begin()); 
        // The bitmap for 'regno' is already set (by varToSpill).
        // It will now be associated with 'var' (or remain set if 'var' is null).
        // No need to call bitmapSet(regno) again if it was already set,
        // but for clarity or if the spilled reg was somehow marked free, call it.
        bitmapSet(regno); 
    }

    // Associate the allocated register with 'var' if 'var' is not null
    if (var) {
        if (regno != -1) { // Only if allocation/spill was successful
            var->setLoadRegId(regno);
            // Remove 'var' if it was already in regValues (e.g., from a previous allocation)
            // to ensure it's added at the end (most recently used).
            auto it = std::find(regValues.begin(), regValues.end(), var);
            if (it != regValues.end()) {
                regValues.erase(it);
            }
            regValues.push_back(var);
            minic_log(LOG_DEBUG, "Allocator: Associated reg %d with Var %s. regValues size: %zu.",
                      regno, var_details.c_str(), regValues.size());
        } else {
            // Allocation failed, var->setLoadRegId should not be called with -1 here
            // as 'regno' would be -1 from the spill failure or no-free-regs case.
            // The error log for this is handled where regno remains -1.
        }
    } else if (regno != -1) {
        // Register allocated anonymously (var is nullptr)
        minic_log(LOG_DEBUG, "Allocator: Reg %d allocated (or kept allocated after spill) but no var provided to associate. Bitmap updated.", regno);
    }

    // Final status log
    if (regno == -1 && var != nullptr) {
         minic_log(LOG_ERROR, "Allocator: FINAL - Allocation FAILED for Var %s.", var_details.c_str());
    } else if (regno != -1 && var != nullptr) {
        minic_log(LOG_INFO, "Allocator: FINAL - Successfully allocated reg %d to Var %s.",
                  regno, var_details.c_str());
    } else if (regno != -1 && var == nullptr) {
        minic_log(LOG_INFO, "Allocator: FINAL - Successfully allocated reg %d anonymously.", regno);
    }


    return regno;
}

void SimpleRegisterAllocator::Allocate(int32_t no) {
    minic_log(LOG_DEBUG, "Allocator::Allocate (force) called for Reg: %d", no);
    if (no < 0 || no >= PlatformArm32::maxUsableRegNum) {
        minic_log(LOG_ERROR, "Allocator: Force allocate called with invalid reg %d.", no);
        return;
    }

    if (regBitmap.test(no)) {
        minic_log(LOG_DEBUG, "Allocator: Reg %d is busy, freeing it first.", no);
        free(no); // This will disassociate any Value and update regValues and bitmap
    }
    bitmapSet(no); // Mark as occupied
    minic_log(LOG_INFO, "Allocator: Force allocated reg %d.", no);
}

void SimpleRegisterAllocator::free(Value * var) {
    if (!var) return;
    std::string var_details = getValueDetails(var);
    minic_log(LOG_DEBUG, "Allocator::free called for Var: %s", var_details.c_str());

    int32_t reg_to_free = var->getLoadRegId();
    if (reg_to_free != -1) {
        if (reg_to_free < 0 || reg_to_free >= PlatformArm32::maxUsableRegNum) {
            minic_log(LOG_ERROR, "Allocator: Var %s has invalid regId %d to free. Ignoring.", 
                      var_details.c_str(), reg_to_free);
            var->setLoadRegId(-1); // Still disassociate from bad ID
            // Do not touch regBitmap if reg_to_free is out of bounds
            // Remove from regValues if present
            auto it = std::find(regValues.begin(), regValues.end(), var);
            if (it != regValues.end()) {
                regValues.erase(it);
            }
            return;
        }

        minic_log(LOG_INFO, "Allocator: Freeing reg %d previously held by Var %s.", 
                  reg_to_free, var_details.c_str());
        regBitmap.reset(reg_to_free);
        var->setLoadRegId(-1);

        auto it = std::find(regValues.begin(), regValues.end(), var);
        if (it != regValues.end()) {
            regValues.erase(it);
        } else {
            minic_log(LOG_WARNING, "Allocator: Var %s was freed from reg %d, but not found in regValues list.",
                      var_details.c_str(), reg_to_free);
        }
    } else {
        minic_log(LOG_DEBUG, "Allocator: Var %s was not associated with any register (loadRegId is -1). Nothing to free.",
                  var_details.c_str());
    }
}

void SimpleRegisterAllocator::free(int32_t no) {
    minic_log(LOG_DEBUG, "Allocator::free called for Reg: %d", no);
    if (no == -1 || no >= PlatformArm32::maxUsableRegNum) { // Check upper bound too
        minic_log(LOG_WARNING, "Allocator: Attempt to free invalid regId %d.", no);
        return;
    }

    if (!regBitmap.test(no)) {
        minic_log(LOG_WARNING, "Allocator: Attempt to free reg %d which is already free in bitmap.", no);
    }
    regBitmap.reset(no); // Mark as free in bitmap

    // Find and update the Value associated with this register in regValues
    auto pIter = regValues.begin();
    while (pIter != regValues.end()) {
        if ((*pIter) && (*pIter)->getLoadRegId() == no) {
            std::string val_details = getValueDetails(*pIter);
            minic_log(LOG_INFO, "Allocator: Disassociating Var %s from freed reg %d.", val_details.c_str(), no);
            (*pIter)->setLoadRegId(-1);
            pIter = regValues.erase(pIter); // Erase and get next iterator
        } else {
            ++pIter;
        }
    }
     minic_log(LOG_INFO, "Allocator: Reg %d marked as free.", no);
}

void SimpleRegisterAllocator::bitmapSet(int32_t no) {
    if (no >= 0 && no < PlatformArm32::maxUsableRegNum) {
        regBitmap.set(no);
        usedBitmap.set(no); // Track that this register was used at some point
    } else {
        minic_log(LOG_ERROR, "Allocator: Attempt to bitmapSet invalid regId %d.", no);
    }
}