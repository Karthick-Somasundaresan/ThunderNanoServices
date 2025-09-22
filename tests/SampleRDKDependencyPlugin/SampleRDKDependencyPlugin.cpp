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
#include "SampleRDKDependencyPlugin.h"
#include <interfaces/IDictionary.h>

namespace WPEFramework {

namespace Plugin {

    namespace {
        static Metadata<SampleRDKDependencyPlugin> metadata(
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

    const string SampleRDKDependencyPlugin::Initialize(PluginHost::IShell *service)
    {
        printf("Inside %s %lu\n", __FUNCTION__, pthread_self());
        string message{};
        uint8_t initFailCount = 0;

        ASSERT(service != nullptr);
        ASSERT(_service == nullptr);
        ASSERT(_sampleRDKPluginImplementation == nullptr);
        ASSERT(_connectionId == 0);

        Config config;
        _service = service;
        config.FromString(service->ConfigLine());
        _service->AddRef();
        _service->Register(&_notification);
        Exchange::IDictionary* storage = service->QueryInterfaceByCallsign<Exchange::IDictionary>(_T("Dictionary"));
        if (storage == nullptr) {
            message = _T("Unable to get the Dictionary");
        }
        else {
            string failCountStr;
#if 0
            Core::hresult retVal = storage->Get("/stormtest-" + service->Callsign(), "failCount", failCountStr);
            if(retVal != Core::ERROR_NONE) {
                if (retVal == Core::ERROR_UNKNOWN_KEY) {
                       Core::hresult result = storage->Set("/stormtest-" + service->Callsign(), "failCount", std::to_string(0));
                        if (result != Core::ERROR_NONE) {
                            TRACE(Trace::Error, (_T("Unable to update count in dictionary\n")));
                            printf("Unable to set initial count in dictionary: error: %d\n", result);
                        }
                } else {
                    printf("[SampleRDKDep] [%s:%d:%s] Get failed with retVal:%d\n", __FILE__, __LINE__, __FUNCTION__, retVal);
                }
            } else {
                char* end;
                initFailCount =  std::strtol(failCountStr.c_str(), &end, 10);
                if (*end !='\0') {
                    message = _T("Not a valid value from Dictionary");
                }
            }
#else

            bool retVal = storage->Get("/stormtest-" + service->Callsign(), "failCount", failCountStr);
            if(retVal == false ) {
                bool result = storage->Set("/stormtest-" + service->Callsign(), "failCount", std::to_string(0));
                if (result == false) {
                    TRACE(Trace::Error, (_T("Unable to update count in dictionary\n")));
                    printf("Unable to set initial count in dictionary: error: %d\n", result);
                }
            } else {
                char* end;
                initFailCount =  std::strtol(failCountStr.c_str(), &end, 10);
                if (*end !='\0') {
                    message = _T("Not a valid value from Dictionary");
                }
            }
#endif
        }
        printf("[SampleRDKDep][%s:%d:%s] (%lu) Current %d config :%d\n", __FILE__,__LINE__, __FUNCTION__, pthread_self(), initFailCount, config.FailOnInitialize.Value());
        if (message.empty() == true) {

            if (initFailCount <= config.FailOnInitialize.Value()) {
                message = _T("Init failed as per configuration");
                initFailCount++;
#if 0
                Core::hresult result = storage->Set("/stormtest-" + service->Callsign(), "failCount",std::to_string(initFailCount));
                if (result != Core::ERROR_NONE) {
                    TRACE(Trace::Error, (_T("Unable to update count in dictionary\n")));
                }
#else
                bool result = storage->Set("/stormtest-" + service->Callsign(), "failCount",std::to_string(initFailCount));
                if (result == false) {
                    TRACE(Trace::Error, (_T("Unable to update count in dictionary\n")));
                }
#endif
            }
        }
        if(storage != nullptr)
        storage->Release();

        if(message.empty() == true) {
            _sampleRDKPluginImplementation = _service->Root<WPEFramework::QualityAssurance::ISampleRDKDependencyPlugin>(_connectionId, 2000, _T("SampleRDKDependencyPluginImplementation"));
            ASSERT(_sampleRDKPluginImplementation != nullptr);

            if (_sampleRDKPluginImplementation == nullptr) {
                message = _T("SampleRDKDependencyPlugin could not be instantiated");
            } else {
                _sampleRDKPluginImplementation->SetDelay(config.PropertyResponseDelay.Value());
                _sampleRDKPluginImplementation->SetNotificationFrequency(config.NotificationFrequency.Value());
                _sampleRDKPluginImplementation->Register(&_notification);
                // QualityAssurance::JSampleRDKDependencyPlugin::Register(*this, _sampleRDKPluginImplementation);
            }
        }
        return (message);
    }

    void SampleRDKDependencyPlugin::Deinitialize(PluginHost::IShell* service VARIABLE_IS_NOT_USED)
    {
        if (_service != nullptr) {
            ASSERT(_service == service);

            if (_sampleRDKPluginImplementation != nullptr) {
                _sampleRDKPluginImplementation->Unregister(&_notification);
                //QualityAssurance::JSampleRDKDependencyPlugin::Unregister(*this);

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

    string SampleRDKDependencyPlugin::Information() const
    {
        return {};
    }

    void SampleRDKDependencyPlugin::Deactivated(RPC::IRemoteConnection* connection)
    {
        if (connection->Id() == _connectionId) {
            ASSERT(_service != nullptr);

            Core::IWorkerPool::Instance().Submit(PluginHost::IShell::Job::Create(_service,
                PluginHost::IShell::DEACTIVATED, PluginHost::IShell::FAILURE));
        }
    }

} // namespace Plugin

}
