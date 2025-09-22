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
#include "SampleRDKPlugin.h"

namespace WPEFramework {

namespace Plugin {

    namespace {
        static Metadata<SampleRDKPlugin> metadata(
            // Version
            1, 0, 0,
            // Preconditions
            { },
            // Terminations
            { },
            // Controls
            { }
        );
    }

    const string SampleRDKPlugin::Initialize(PluginHost::IShell *service)
    {
        string message{};
        Config config;

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_sampleRDKPluginImplementation == nullptr);
        ASSERT(_connectionId == 0);
        config.FromString(service->ConfigLine());

        _service = service;
        _service->AddRef();
        _service->Register(&_notification);
        if (config.StartDelay.Value() != 0)
        {
            SleepMs(config.StartDelay.Value());
        }

        _sampleRDKPluginImplementation = _service->Root<WPEFramework::QualityAssurance::ISampleRDKPlugin>(_connectionId, 2000, _T("SampleRDKPluginImplementation"));
        ASSERT(_sampleRDKPluginImplementation != nullptr);

        if (_sampleRDKPluginImplementation == nullptr) {
            message = _T("SampleRDKPlugin could not be instantiated");
        } else {
            _sampleRDKPluginImplementation->Configure(service);
            _sampleRDKPluginImplementation->Register(&_notification);
            QualityAssurance::JSampleRDKPlugin::Register(*this, _sampleRDKPluginImplementation);
        }

        return (message);
    }

    void SampleRDKPlugin::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED)
    {
        if (_service != nullptr) {
            ASSERT(_service == service);

            if (_sampleRDKPluginImplementation != nullptr) {
                _sampleRDKPluginImplementation->Unregister(&_notification);
                QualityAssurance::JSampleRDKPlugin::Unregister(*this);

                RPC::IRemoteConnection* connection(_service->RemoteConnection(_connectionId));
                VARIABLE_IS_NOT_USED const uint32_t result = _sampleRDKPluginImplementation->Release();
                ASSERT( (result == Core::ERROR_CONNECTION_CLOSED) || (result == Core::ERROR_DESTRUCTION_SUCCEEDED));

                _sampleRDKPluginImplementation = nullptr;

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

    string SampleRDKPlugin::Information() const
    {
        return {};
    }

    void SampleRDKPlugin::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin

}
