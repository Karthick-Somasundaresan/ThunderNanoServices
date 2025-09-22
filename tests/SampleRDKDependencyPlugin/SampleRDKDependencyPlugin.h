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

#pragma once

#include "Module.h"
#include <qa_interfaces/ISampleRDKDependencyPlugin.h>
#include <qa_interfaces/json/JSampleRDKDependencyPlugin.h>

namespace WPEFramework {

namespace Plugin {

    class SampleRDKDependencyPlugin : public PluginHost::IPlugin
               , public PluginHost::JSONRPC {
    private:
        class Notification : public RPC::IRemoteConnection::INotification
                           , public QualityAssurance::ISampleRDKDependencyPlugin::INotification {
        public:
            Notification(SampleRDKDependencyPlugin& parent)
                : _parent(parent) {
            }
            ~Notification() override = default;

            Notification() = delete;
            Notification(const Notification&) = delete;
            Notification& operator=(const Notification&) = delete;

        public:
            // RPC::IRemoteConnection::INotification overrides
            void Activated(RPC::IRemoteConnection*) override
            {
            }
            void Deactivated(RPC::IRemoteConnection* connection) override
            {
                _parent.Deactivated(connection);
            }

        public:
            // Exchange::ISampleRDKDependencyPlugin::INotification overrides
            
            void FastNotification()
            {
                QualityAssurance::JSampleRDKDependencyPlugin::Event::FastNotification(_parent);
            }
            void SlowNotification()
            {
                QualityAssurance::JSampleRDKDependencyPlugin::Event::SlowNotification(_parent);
            }

        public:
            BEGIN_INTERFACE_MAP(Notification)
                INTERFACE_ENTRY(RPC::IRemoteConnection::INotification)
                INTERFACE_ENTRY(QualityAssurance::ISampleRDKDependencyPlugin::INotification)
            END_INTERFACE_MAP

        private:
            SampleRDKDependencyPlugin& _parent;
        }; // class Notification

        class Config : public Core::JSON::Container
        {
            public:
                Config(const Config&) = delete;
                Config& operator=(Config&) = delete;
                ~Config() = default;
                Config()
                : Core::JSON::Container()
                , FailOnInitialize(0)
                , PropertyResponseDelay(0)
                , NotificationFrequency(1000)
                {
                    Add(_T("failoninitialize"), &FailOnInitialize);
                    Add(_T("propertyresponsedelay"), &PropertyResponseDelay);
                    Add(_T("notificationfrequency"), &NotificationFrequency);
                }


            public:
                Core::JSON::DecUInt8 FailOnInitialize;
                Core::JSON::DecUInt16 PropertyResponseDelay;
                Core::JSON::DecUInt16 NotificationFrequency;
        };
    public:
        SampleRDKDependencyPlugin()
            : _service(nullptr)
            , _notification(*this)
            , _connectionId(0)
            , _sampleRDKPluginImplementation(nullptr)
        {
        }
        ~SampleRDKDependencyPlugin() override = default;

        SampleRDKDependencyPlugin(const SampleRDKDependencyPlugin&) = delete;
        SampleRDKDependencyPlugin &operator=(const SampleRDKDependencyPlugin&) = delete;

    public:
        // PluginHost::IPlugin overrides
        const string Initialize(PluginHost::IShell *service) override;
        void Deinitialize(PluginHost::IShell *service) override;
        string Information() const override;

    private:
        void Deactivated(RPC::IRemoteConnection* connection);

    public:
        BEGIN_INTERFACE_MAP(SampleRDKDependencyPlugin)
            INTERFACE_ENTRY(PluginHost::IPlugin)
            INTERFACE_ENTRY(PluginHost::IDispatcher)
            INTERFACE_AGGREGATE(QualityAssurance::ISampleRDKDependencyPlugin, _sampleRDKPluginImplementation)
        END_INTERFACE_MAP

    private:
        PluginHost::IShell *_service;
        Core::Sink<Notification> _notification;
        uint32_t _connectionId;
        QualityAssurance::ISampleRDKDependencyPlugin *_sampleRDKPluginImplementation;
    }; // class SampleRDKDependencyPlugin

} // namespace Plugin

}
