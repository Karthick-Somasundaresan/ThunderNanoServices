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

#include <qa_interfaces/ISubsystemController.h>
#include <interfaces/IConfiguration.h>

namespace WPEFramework {

namespace Plugin {

    class SubsystemControllerImplementation: public QualityAssurance::ISubsystemController, public WPEFramework::Exchange::IConfiguration {
    private:
        class Config: public Core::JSON::Container {
        private:
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;

        public:
            Config() : SubsystemStartDelay(0)
            {
                Add(_T("subsystemstartdelay"), &SubsystemStartDelay);
            }

            ~Config()
            {
            }

        public:
            Core::JSON::DecUInt16 SubsystemStartDelay;
        };
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

        class LocationInfo :  public PluginHost::ISubSystem::ILocation {
        public:
            LocationInfo(const LocationInfo&) = default;
            LocationInfo(LocationInfo&&) = default;
            LocationInfo& operator=(const LocationInfo&) = default;
            LocationInfo& operator=(LocationInfo&&) = default;

            LocationInfo()
                : _timeZone()
                , _country()
                , _region()
                , _city()
                , _latitude(std::numeric_limits<int32_t>::min())
                , _longitude(std::numeric_limits<int32_t>::min())
            {
            }
            LocationInfo(int32_t latitude, int32_t longitude)
                : _timeZone()
                , _country()
                , _region()
                , _city()
                , _latitude(latitude)
                , _longitude(longitude)
            {
            }
            ~LocationInfo() override = default;

        public:
            BEGIN_INTERFACE_MAP(Location)
            INTERFACE_ENTRY(PluginHost::ISubSystem::ILocation)
            END_INTERFACE_MAP

        public:
            string TimeZone() const override { return _timeZone; }
            void TimeZone(const string& timezone) { _timeZone = timezone; }
            string Country() const override { return _country; }
            void Country(const string& country) { _country = country; }
            string Region() const override { return _region; }
            void Region(const string& region) { _region = region; }
            string City() const override { return _city; }
            void City(const string& city) { _city = city; }
            int32_t Latitude() const override { return _latitude; }
            void Latitude(const int32_t latitude) { _latitude = latitude; }
            int32_t Longitude() const override { return _longitude; }
            void Longitude(const int32_t longitude) { _longitude = longitude; }

        private:
            string _timeZone;
            string _country;
            string _region;
            string _city;
            int32_t _latitude; 
            int32_t _longitude;
        };
    public:
        SubsystemControllerImplementation()
            : _service(nullptr)
            , _lock()
            , _observers()
            , _decoupledJob()
            , _locationInfo()
        {
        }
        ~SubsystemControllerImplementation() override
        {
            if (_service != nullptr) {
                _service->Release();
            }
        }

        SubsystemControllerImplementation(const SubsystemControllerImplementation&) = delete;
        SubsystemControllerImplementation& operator=(const SubsystemControllerImplementation&) = delete;

    public:
        // ISubsystemController overrides
          // @brief API to enable or disable subsystem
          // @detail enable = True - enables the subsystem, False - disables subsystem
        uint32_t EnableSubsystem(const bool& enable) override {
            return Core::ERROR_NONE;
        }

        uint32_t SubsystemStatus(bool& status /* @out */ ) const override {
            return Core::ERROR_NONE;
        }

        uint32_t Configure(PluginHost::IShell* framework) override {
            printf("Inside Configure in impl side\n");
            _service = framework;
            _service->AddRef();
            printf("Inside Configure Before ConfigLine\n");
            Config config;
            config.FromString(_service->ConfigLine());
            printf("Inside Configure After ConfigLine\n");
            if(config.SubsystemStartDelay.Value() == 0)
            {
                 PluginHost::ISubSystem* subSystems = _service->SubSystems();
                 if(subSystems != nullptr) {
                    printf("Inside Configure Setting subsystem to on\n");
                    subSystems->Set(PluginHost::ISubSystem::LOCATION, &_locationInfo);
                 }
            }
            printf("Inside Configure End of function\n");
            return Core::ERROR_NONE;

        }
        uint32_t Register(QualityAssurance::ISubsystemController::INotification* notification)
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
        uint32_t Unregister(const QualityAssurance::ISubsystemController::INotification* notification)
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
        BEGIN_INTERFACE_MAP(SubsystemControllerImplementation)
            INTERFACE_ENTRY(QualityAssurance::ISubsystemController)
            INTERFACE_ENTRY(Exchange::IConfiguration)
        END_INTERFACE_MAP

    private:
        PluginHost::IShell* _service;
        mutable Core::CriticalSection _lock;
        std::list<QualityAssurance::ISubsystemController::INotification*> _observers;
        DecoupledJob _decoupledJob;
        Core::Sink<LocationInfo> _locationInfo;
    }; // class SubsystemControllerImplementation

    SERVICE_REGISTRATION(SubsystemControllerImplementation, 1, 0)

} // namespace Plugin

}
