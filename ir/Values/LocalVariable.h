// ir/Values/LocalVariable.h
#pragma once

#include "Value.h"
// #include "IRConstant.h" // 如果 LocalVariable.h 本身不直接使用 IRConstant，可以考虑是否必要
#include "Common.h" // For minic_log，如果 getMemoryAddr 等内联日志需要
#include <string>          // For std::string
#include <cstdint>         // For int32_t, int64_t

class LocalVariable : public Value {
    friend class Function;

private:
    explicit LocalVariable(Type * _type, std::string _name, int32_t _scope_level)
        : Value(_type), scope_level(_scope_level) {
        this->name = _name;
        // regId, offset, baseRegNo, loadRegNo 都有类内默认初始化
    }

    int scope_level = -1;
    int32_t regId = -1;       // "Fixed" register allocation for this local var
    int32_t offset = 0;       // Stack offset relative to baseRegNo
    int32_t baseRegNo = -1;   // Base register for stack access (e.g., fp)
    // std::string baseRegName; // 可以考虑移除，因为 baseRegNo 更常用且类型安全
    int32_t loadRegNo = -1;   // Temp register used by SimpleRegisterAllocator to load this var

public:
    [[nodiscard]] int32_t getScopeLevel() const override {
        return scope_level;
    }

    // Returns the "fixed" register ID if this variable is permanently in a register.
    // This might be different from a temporary register used for loading (loadRegNo).
    [[nodiscard]] int32_t getRegId() override {
        return regId;
    }
    // Setter for the "fixed" register ID
    void setRegId(int32_t id) override { // Added override, assuming Value::setRegId is virtual
        this->regId = id;
    }


    [[nodiscard]] bool getMemoryAddr(int32_t * _regId = nullptr, int64_t * _offset = nullptr) override {
        minic_log(LOG_DEBUG, "LocalVar '%s' (IR: '%s', Ptr: %p): getMemoryAddr() called. Internal baseRegNo: %d, offset: %d.", 
                  getName().c_str(), getIRName().c_str(), (void*)this, 
                  this->baseRegNo, this->offset);
        if (this->baseRegNo == -1) {
            return false;
        }
        if (_regId) {
            *_regId = this->baseRegNo;
        }
        if (_offset) {
            *_offset = static_cast<int64_t>(this->offset); // Ensure correct type for output param
        }
        return true;
    }

    void setMemoryAddr(int32_t _regId, int64_t _offset_param) {
        this->baseRegNo = _regId;
        // Be careful if _offset_param can exceed int32_t range for this->offset
        if (_offset_param < INT32_MIN || _offset_param > INT32_MAX) {
            minic_log(LOG_ERROR, "LocalVar '%s': Large offset %lld provided, may truncate in int32_t member.",
                      getIRName().c_str(), (long long)_offset_param);
        }
        this->offset = static_cast<int32_t>(_offset_param); 
        
        minic_log(LOG_DEBUG, "LocalVar '%s' (IR: '%s', Ptr: %p): setMemoryAddr called. baseRegNo set to %d, offset_param was %lld, member offset set to %d.", 
                  getName().c_str(), getIRName().c_str(), (void*)this, 
                  this->baseRegNo, (long long)_offset_param, this->offset);
    }

    // These are for SimpleRegisterAllocator to manage temporary loading into a register
    [[nodiscard]] int32_t getLoadRegId() override {
        return this->loadRegNo;
    }

    void setLoadRegId(int32_t tempRegId) override {
        // minic_log(LOG_DEBUG, "LocalVar %s (IR: %s): setLoadRegId to %d (was %d)", 
        //           getName().c_str(), getIRName().c_str(), tempRegId, this->loadRegNo);
        this->loadRegNo = tempRegId;
    }

    [[nodiscard]] int32_t getBaseRegNoForDebug() const { return baseRegNo; }
    [[nodiscard]] int32_t getOffsetForDebug() const { return offset; }
};