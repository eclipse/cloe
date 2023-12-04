local cloe = require("cloe")

-- From: config_nop_infinite.json
cloe.apply_stack {
  version = "4",
  simulators = {
    { binding = "nop" }
  },
  vehicles = {
    {
      name = "default",
      from = {
        simulator = "nop",
        index = 0
      },
      components = {
        ["cloe::speedometer"] = {
          binding = "speedometer",
          name = "default_speed",
          from = "cloe::gndtruth_ego_sensor"
        },
        ["cloe::default_world_sensor"] = {
          binding = "noisy_object_sensor",
          name = "noisy_object_sensor",
          from = "cloe::default_world_sensor",
          args = {
            noise = {
              {
                target = "translation",
                distribution = {
                  binding = "normal",
                  mean = 0.0,
                  std_deviation = 0.3
                }
              }
            }
          }
        }
      }
    }
  }
}

-- From: controller_basic.json
cloe.apply_stack {
  version = "4",
  controllers = {
    { binding = "basic", vehicle = "default" }
  },
  triggers = {
    { action = { actions = { "basic/hmi=!enable" }, name = "bundle" }, event = "start" },
    { action = "basic/hmi=enable", event = "next=1" },
    { action = "basic/hmi=resume", event = "time=5" },
    { action = "basic/hmi=!resume", event = "time=5.5" },
    { action = {
        name = "insert",
        triggers = { {
            action = "basic/hmi=plus",
            event = "next"
          }, {
            action = "basic/hmi=!plus",
            event = "next=1"
          } }
      },
      event = "time=6",
      label = "Push and release basic/hmi=plus"
    }
  },
}

-- From: controller_virtue.json
cloe.apply_stack {
  version = "4",
  controllers = {
    { binding = "virtue", vehicle = "default" }
  },
}

-- From: config_nop_smoketest.json
cloe.apply_stack {
  version = "4",
  server = {
    listen = false,
    listen_port = 23456
  },
  triggers = {
    { action = "fail", event = "virtue/failure" },
    { action = "fail", event = "default_speed/kmph=>0.0", label = "Vehicle default should never move with the nop binding." },
    { action = "log=info: Running nop/basic smoketest.", event = "start" },
    { action = "realtime_factor=-1", event = "start" },
    { action = "succeed", event = "time=60" },
  },
}
