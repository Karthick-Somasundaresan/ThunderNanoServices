/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 Metrological
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Module.h"
#include "SubsystemController.h"
#include <interfaces/IConfiguration.h>

namespace WPEFramework {

namespace Plugin {

    namespace {
        static Metadata<SubsystemController> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            { },
            // Terminations
            { },
            // Controls
            { subsystem::LOCATION}
        );
    }

    const string SubsystemController::Initialize(PluginHost::IShell *service)
    {
        string message{};

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_subsystemControllerImplementation == nullptr);
        ASSERT(_connectionId == 0);

        _service = service;
        _service->AddRef();
        _service->Register(&_notification);

        printf("Before service Root\n");
        _subsystemControllerImplementation = _service->Root<WPEFramework::QualityAssurance::ISubsystemController>(_connectionId, 2000, _T("SubsystemControllerImplementation"));
        ASSERT(_subsystemControllerImplementation != nullptr);
        printf("After service Root\n");

        if (_subsystemControllerImplementation == nullptr) {
            message = _T("SubsystemController could not be instantiated");
        } else {
            Exchange::IConfiguration *configure = _subsystemControllerImplementation->QueryInterface<Exchange::IConfiguration>();
            printf("Calling Configure\n");
            configure->Configure(service);
            configure->Release();
            _subsystemControllerImplementation->Register(&_notification);
            QualityAssurance::JSubsystemController::Register(*this, _subsystemControllerImplementation);
        }

        return (message);
    }

    void SubsystemController::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED)
    {
        if (_service != nullptr) {
            ASSERT(_service == service);

            if (_subsystemControllerImplementation != nullptr) {
                _subsystemControllerImplementation->Unregister(&_notification);
                QualityAssurance::JSubsystemController::Unregister(*this);

                RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));
                VARIABLE_IS_NOT_USED const uint32_t result = _subsystemControllerImplementation->Release();
                ASSERT( (result == Core::ERROR_CONNECTION_CLOSED) || (result == Core::ERROR_DESTRUCTION_SUCCEEDED));

                _subsystemControllerImplementation = nullptr;

                if (connection != nullptr) {
                    connection->Terminate();
                    connection->Release();
                }
            }

            _service->Unregister(&_notification);

            _connectionId = 0;

            _service->Release();
            _service = nullptr;
        }
    }

    string SubsystemController::Information() const
    {
        return {};
    }

    void SubsystemController::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin

}
