/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/built_ins/built_ins.h"
#include "shared/source/execution_environment/root_device_environment.h"
#include "shared/test/common/fixtures/device_fixture.h"
#include "shared/test/common/helpers/debug_manager_state_restore.h"
#include "shared/test/common/helpers/memory_management.h"
#include "shared/test/common/helpers/ult_hw_config.h"
#include "shared/test/common/mocks/mock_builtinslib.h"
#include "shared/test/common/mocks/mock_compiler_interface_spirv.h"
#include "shared/test/common/test_macros/hw_test.h"

#include "level_zero/core/source/builtin/builtin_functions_lib_impl.h"
#include "level_zero/core/source/device/device_imp.h"
#include "level_zero/core/test/unit_tests/fixtures/device_fixture.h"
#include "level_zero/core/test/unit_tests/mocks/mock_device_for_built_ins.h"
#include "level_zero/core/test/unit_tests/mocks/mock_device_for_spirv.h"
#include "level_zero/core/test/unit_tests/mocks/mock_kernel.h"
#include "level_zero/core/test/unit_tests/mocks/mock_module.h"

#include "gtest/gtest.h"

namespace L0 {
namespace ult {
class BuiltinFunctionsLibFixture : public DeviceFixture {
  public:
    struct MockBuiltinFunctionsLibImpl : BuiltinFunctionsLibImpl {
        using BuiltinFunctionsLibImpl::builtins;
        using BuiltinFunctionsLibImpl::ensureInitCompletion;
        using BuiltinFunctionsLibImpl::getFunction;
        using BuiltinFunctionsLibImpl::imageBuiltins;
        using BuiltinFunctionsLibImpl::initAsyncComplete;
        MockBuiltinFunctionsLibImpl(L0::Device *device, NEO::BuiltIns *builtInsLib) : BuiltinFunctionsLibImpl(device, builtInsLib) {
            mockModule = std::unique_ptr<Module>(new Mock<Module>(device, nullptr));
        }
        std::unique_ptr<BuiltinData> loadBuiltIn(NEO::EBuiltInOps::Type builtin, const char *builtInName) override {
            std::unique_ptr<Kernel> mockKernel(new Mock<::L0::KernelImp>());

            return std::unique_ptr<BuiltinData>(new BuiltinData{mockModule.get(), std::move(mockKernel)});
        }
        std::unique_ptr<Module> mockModule;
    };

    void setUp() {
        DeviceFixture::setUp();
        mockDevicePtr = std::unique_ptr<MockDeviceForSpv<false, false>>(new MockDeviceForSpv<false, false>(device->getNEODevice(), device->getNEODevice()->getExecutionEnvironment(), driverHandle.get()));
        mockBuiltinFunctionsLibImpl.reset(new MockBuiltinFunctionsLibImpl(mockDevicePtr.get(), neoDevice->getBuiltIns()));
        mockBuiltinFunctionsLibImpl->ensureInitCompletion();
        EXPECT_TRUE(mockBuiltinFunctionsLibImpl->initAsyncComplete);
    }
    void tearDown() {
        mockBuiltinFunctionsLibImpl.reset();
        DeviceFixture::tearDown();
    }

    std::unique_ptr<MockBuiltinFunctionsLibImpl> mockBuiltinFunctionsLibImpl;
    std::unique_ptr<MockDeviceForSpv<false, false>> mockDevicePtr;
};

using TestBuiltinFunctionsLibImpl = Test<BuiltinFunctionsLibFixture>;

HWTEST_F(TestBuiltinFunctionsLibImpl, givenImageSupportThenEachBuiltinImageFunctionsIsLoadedOnlyOnce) {
    L0::Kernel *initializedImageBuiltins[static_cast<uint32_t>(ImageBuiltin::COUNT)];

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(ImageBuiltin::COUNT); builtId++) {
        EXPECT_EQ(nullptr, mockBuiltinFunctionsLibImpl->imageBuiltins[builtId]);
    }

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(ImageBuiltin::COUNT); builtId++) {
        EXPECT_NE(nullptr, mockBuiltinFunctionsLibImpl->getImageFunction(static_cast<L0::ImageBuiltin>(builtId)));
        EXPECT_NE(nullptr, mockBuiltinFunctionsLibImpl->imageBuiltins[builtId]);
        initializedImageBuiltins[builtId] = mockBuiltinFunctionsLibImpl->imageBuiltins[builtId]->func.get();
    }

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(ImageBuiltin::COUNT); builtId++) {
        EXPECT_EQ(initializedImageBuiltins[builtId],
                  mockBuiltinFunctionsLibImpl->getImageFunction(static_cast<L0::ImageBuiltin>(builtId)));
    }
}

HWTEST_F(TestBuiltinFunctionsLibImpl, givenImageSupportAndWrongIdWhenCallingBuiltinImageFunctionThenExceptionIsThrown) {
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(ImageBuiltin::COUNT); builtId++) {
        EXPECT_EQ(nullptr, mockBuiltinFunctionsLibImpl->imageBuiltins[builtId]);
    }

    uint32_t builtId = static_cast<uint32_t>(ImageBuiltin::COUNT) + 1;
    EXPECT_THROW(mockBuiltinFunctionsLibImpl->initBuiltinImageKernel(static_cast<L0::ImageBuiltin>(builtId)), std::exception);
}

HWTEST_F(TestBuiltinFunctionsLibImpl, givenCallsToGetFunctionThenEachBuiltinFunctionsIsLoadedOnlyOnce) {
    L0::Kernel *initializedBuiltins[static_cast<uint32_t>(Builtin::COUNT)];

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        EXPECT_EQ(nullptr, mockBuiltinFunctionsLibImpl->builtins[builtId]);
    }

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        EXPECT_NE(nullptr, mockBuiltinFunctionsLibImpl->getFunction(static_cast<L0::Builtin>(builtId)));
        EXPECT_NE(nullptr, mockBuiltinFunctionsLibImpl->builtins[builtId]);
        initializedBuiltins[builtId] = mockBuiltinFunctionsLibImpl->builtins[builtId]->func.get();
    }

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        EXPECT_EQ(initializedBuiltins[builtId],
                  mockBuiltinFunctionsLibImpl->getFunction(static_cast<L0::Builtin>(builtId)));
    }
}

HWTEST_F(TestBuiltinFunctionsLibImpl, givenCallToBuiltinFunctionWithWrongIdThenExceptionIsThrown) {
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        EXPECT_EQ(nullptr, mockBuiltinFunctionsLibImpl->builtins[builtId]);
    }
    uint32_t builtId = static_cast<uint32_t>(Builtin::COUNT) + 1;
    EXPECT_THROW(mockBuiltinFunctionsLibImpl->initBuiltinKernel(static_cast<L0::Builtin>(builtId)), std::exception);
}

HWTEST_F(TestBuiltinFunctionsLibImpl, whenCreateBuiltinFunctionsLibThenImmediateFillIsLoaded) {
    struct MockBuiltinFunctionsLibImpl : public BuiltinFunctionsLibImpl {
        using BuiltinFunctionsLibImpl::BuiltinFunctionsLibImpl;
        using BuiltinFunctionsLibImpl::builtins;
        using BuiltinFunctionsLibImpl::ensureInitCompletion;
        using BuiltinFunctionsLibImpl::initAsyncComplete;
    };

    EXPECT_TRUE(mockBuiltinFunctionsLibImpl->initAsyncComplete);
    VariableBackup<UltHwConfig> backup(&ultHwConfig);
    ultHwConfig.useinitBuiltinsAsyncEnabled = true;
    MockBuiltinFunctionsLibImpl lib(device, device->getNEODevice()->getBuiltIns());
    EXPECT_FALSE(lib.initAsyncComplete);
    lib.ensureInitCompletion();
    EXPECT_TRUE(lib.initAsyncComplete);
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        if (builtId == static_cast<uint32_t>(Builtin::FillBufferImmediate)) {
            EXPECT_NE(nullptr, lib.builtins[builtId]);
        } else {
            EXPECT_EQ(nullptr, lib.builtins[builtId]);
        }
    }
    uint32_t builtId = static_cast<uint32_t>(Builtin::COUNT) + 1;
    EXPECT_THROW(lib.initBuiltinKernel(static_cast<L0::Builtin>(builtId)), std::exception);

    /* std::async may create a detached thread - completion of the scheduled task can be ensured,
       but there is no way to ensure that actual OS thread exited and its resources are freed */
    MemoryManagement::fastLeaksDetectionMode = MemoryManagement::LeakDetectionMode::TURN_OFF_LEAK_DETECTION;
}

HWTEST_F(TestBuiltinFunctionsLibImpl, givenCompilerInterfaceWhenCreateDeviceAndImageSupportedThenBuiltinsImageFunctionsAreLoaded) {
    ze_result_t returnValue = ZE_RESULT_SUCCESS;
    neoDevice->getExecutionEnvironment()->rootDeviceEnvironments[neoDevice->getRootDeviceIndex()]->compilerInterface.reset(new NEO::MockCompilerInterfaceSpirv());
    std::unique_ptr<L0::Device> testDevice(Device::create(device->getDriverHandle(), neoDevice, false, &returnValue));

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(ImageBuiltin::COUNT); builtId++) {
        EXPECT_NE(nullptr, testDevice->getBuiltinFunctionsLib()->getImageFunction(static_cast<L0::ImageBuiltin>(builtId)));
    }
}

HWTEST_F(TestBuiltinFunctionsLibImpl, givenCompilerInterfaceWhenCreateDeviceThenBuiltinsFunctionsAreLoaded) {
    ze_result_t returnValue = ZE_RESULT_SUCCESS;
    neoDevice->getExecutionEnvironment()->rootDeviceEnvironments[neoDevice->getRootDeviceIndex()]->compilerInterface.reset(new NEO::MockCompilerInterfaceSpirv());
    std::unique_ptr<L0::Device> testDevice(Device::create(device->getDriverHandle(), neoDevice, false, &returnValue));

    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        EXPECT_NE(nullptr, testDevice->getBuiltinFunctionsLib()->getFunction(static_cast<L0::Builtin>(builtId)));
    }
}

using BuiltInTestsL0 = Test<NEO::DeviceFixture>;

HWTEST_F(BuiltInTestsL0, givenRebuildPrecompiledKernelsDebugFlagWhenInitFuctionsThenIntermediateCodeForBuiltinsIsRequested) {
    pDevice->incRefInternal();
    DebugManagerStateRestore dgbRestorer;
    NEO::DebugManager.flags.RebuildPrecompiledKernels.set(true);
    MockDeviceForBuiltinTests testDevice(pDevice);
    testDevice.builtins.reset(new BuiltinFunctionsLibImpl(&testDevice, pDevice->getBuiltIns()));
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        testDevice.getBuiltinFunctionsLib()->initBuiltinKernel(static_cast<Builtin>(builtId));
    }

    EXPECT_TRUE(testDevice.createModuleCalled);
}

HWTEST_F(BuiltInTestsL0, givenNotToRebuildPrecompiledKernelsDebugFlagWhenInitFuctionsThenNativeCodeForBuiltinsIsRequested) {
    pDevice->incRefInternal();
    DebugManagerStateRestore dgbRestorer;
    NEO::DebugManager.flags.RebuildPrecompiledKernels.set(false);
    MockDeviceForBuiltinTests testDevice(pDevice);
    L0::Device *testDevicePtr = &testDevice;
    testDevice.builtins.reset(new BuiltinFunctionsLibImpl(testDevicePtr, pDevice->getBuiltIns()));
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        testDevice.getBuiltinFunctionsLib()->initBuiltinKernel(static_cast<Builtin>(builtId));
    }

    EXPECT_TRUE(testDevice.createModuleCalled);
}

HWTEST_F(BuiltInTestsL0, givenBuiltinsWhenInitializingFunctionsThenModulesWithProperTypeAreCreated) {
    pDevice->incRefInternal();
    MockDeviceForBuiltinTests testDevice(pDevice);
    L0::Device *testDevicePtr = &testDevice;
    testDevice.builtins.reset(new BuiltinFunctionsLibImpl(testDevicePtr, pDevice->getBuiltIns()));
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(Builtin::COUNT); builtId++) {
        testDevice.getBuiltinFunctionsLib()->initBuiltinKernel(static_cast<Builtin>(builtId));
    }

    EXPECT_EQ(ModuleType::Builtin, testDevice.typeCreated);
}

HWTEST_F(BuiltInTestsL0, givenDeviceWithUnregisteredBinaryBuiltinWhenGettingBuiltinKernelThenOnlyIntermediateFormatIsAvailable) {
    pDevice->incRefInternal();
    MockDeviceForBuiltinTests testDevice(pDevice);
    L0::Device *testDevicePtr = &testDevice;
    pDevice->getRootDeviceEnvironment().getMutableHardwareInfo()->ipVersion.value += 0xdead;
    testDevice.builtins.reset(new BuiltinFunctionsLibImpl(testDevicePtr, pDevice->getBuiltIns()));
    for (uint32_t builtId = 0; builtId < static_cast<uint32_t>(L0::Builtin::COUNT); builtId++) {
        testDevice.formatForModule = {};
        ASSERT_NE(nullptr, testDevice.getBuiltinFunctionsLib()->getFunction(static_cast<L0::Builtin>(builtId)));
        EXPECT_EQ(ZE_MODULE_FORMAT_IL_SPIRV, testDevice.formatForModule);
    }
}

} // namespace ult
} // namespace L0
