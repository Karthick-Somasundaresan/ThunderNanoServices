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

#include <qa_interfaces/ISampleRDKDependencyPlugin.h>

namespace WPEFramework {

namespace Plugin {

    class SampleRDKDependencyPluginImplementation: public QualityAssurance::ISampleRDKDependencyPlugin {
    private:

    private:
        class DecoupledJob {
            friend Core::ThreadPool::JobType<DecoupledJob&>;

        public:
            DecoupledJob()
                : _job(*this)
            {
            }
            ~DecoupledJob() = default;

            DecoupledJob(const DecoupledJob&) = delete;
            DecoupledJob& operator=(const DecoupledJob&) = delete;

        public:
            void Submit(const std::function<void(void)>& fn, const Core::Time& time = 0)
            {
                _fn = fn;
                _job.Reschedule(time);
            }

        private:
            void Dispatch()
            {
                _fn();
            }

        private:
            Core::WorkerPool::JobType<DecoupledJob&> _job;
            std::function<void(void)> _fn;
        }; // class DecoupledJob


    public:
        SampleRDKDependencyPluginImplementation()
            : _service(nullptr)
            , _lock()
            , _observers()
            , _propertyDelay(0)
            , _notificationFrequency(5000)
            , _decoupledJob()
        {
        }
        ~SampleRDKDependencyPluginImplementation() override
        {
            if (_service != nullptr) {
                _service->Release();
            }
        }

        SampleRDKDependencyPluginImplementation(const SampleRDKDependencyPluginImplementation&) = delete;
        SampleRDKDependencyPluginImplementation& operator=(const SampleRDKDependencyPluginImplementation&) = delete;

    private:
        void Configure(const Config& config)
        {


        }

    public:
        /* virtual */ Core::hresult SetDelay(const uint16_t& delay) override {
            _propertyDelay = delay;
            return Core::ERROR_NONE;
        }
        /* virtual */ Core::hresult SetNotificationFrequency(const uint16_t& freq) override{
            _notificationFrequency = freq;
            return Core::ERROR_NONE;
        }
        uint32_t SlowProperty(const uint8_t & value) override {
            return Core::ERROR_NONE;
        }
        uint32_t SlowProperty(uint8_t& value /* @out */) const override {
            return Core::ERROR_NONE;
        }
        uint32_t FastProperty(const uint8_t & value) override {
            return Core::ERROR_NONE;
        }
        uint32_t FastProperty(uint8_t& value /* @out */) const override {
            return Core::ERROR_NONE;
        }
        uint32_t Register(QualityAssurance::ISampleRDKDependencyPlugin::INotification* notification)
        {
            ASSERT(notification != nullptr);

            uint32_t result = Core::ERROR_BAD_REQUEST;

            _lock.Lock();

            if ((notification != nullptr) && (std::find(_observers.begin(), _observers.end(), notification) == _observers.end())) {
                _observers.push_back(notification);
                notification->AddRef();
                result = Core::ERROR_NONE;
            }

            _lock.Unlock();

            return (result);
        }
        uint32_t Unregister(const QualityAssurance::ISampleRDKDependencyPlugin::INotification* notification)
        {
            ASSERT(notification != nullptr);

            uint32_t result = Core::ERROR_BAD_REQUEST;

            _lock.Lock();

            if (notification != nullptr) {
                auto it = std::find(_observers.begin(), _observers.end(), notification);
                if (it != _observers.end()) {
                    (*it)->Release();
                    _observers.erase(it);
                    result = Core::ERROR_NONE;
                }
            }

            _lock.Unlock();

            return (result);
        }

    public:
        BEGIN_INTERFACE_MAP(SampleRDKDependencyPluginImplementation)
            INTERFACE_ENTRY(QualityAssurance::ISampleRDKDependencyPlugin)
        END_INTERFACE_MAP

    private:
        PluginHost::IShell* _service;
        mutable Core::CriticalSection _lock;
        std::list<QualityAssurance::ISampleRDKDependencyPlugin::INotification*> _observers;
        uint16_t _propertyDelay;
        uint16_t _notificationFrequency;
        DecoupledJob _decoupledJob;
    }; // class SampleRDKDependencyPluginImplementation

    SERVICE_REGISTRATION(SampleRDKDependencyPluginImplementation, 1, 0)

} // namespace Plugin

}
