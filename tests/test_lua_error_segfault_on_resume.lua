local cloe = require("cloe")

cloe.load_stackfile("config_nop_infinite.json")

cloe.schedule {
    on = "start",
    desc = "A demonstration of what works",
    enable = true,
    run = function()
        local test = {}
        local resume_main = function ()
            cloe.log("info", "this should not fail or segfault")
            local ok, result = coroutine.resume(test.co)
            if not ok then
                error(result)
            end
        end

        test.co = coroutine.create(function()
            -- Running scheduler.insert inside this function leads to segfault...
            -- Not sure why, so we need to yield the function first.
            coroutine.yield(function()
                cloe.scheduler.insert {
                    event = "next",
                    action = resume_main,
                    action_source = "simplified scheduler.insert",
                }
            end)
            cloe.log("info", "awesome, it works")
        end)
        local ok, result = coroutine.resume(test.co)
        if not ok then
            error(result)
        elseif type(result) == "function" then
            result()
        end
    end
}

cloe.schedule {
    on = "start",
    desc = "A demonstration of what leads to segfault",
    enable = true,
    run = function()
        local test = {}
        local resume_main = function ()
            cloe.log("info", "this should not fail or segfault, but it does")
            local ok, result = coroutine.resume(test.co)
            if not ok then
                error(result)
            end
        end

        test.co = coroutine.create(function()
            -- Running scheduler.insert inside this function leads to segfault...
            -- Not sure why, so we need to yield the function first.
            cloe.scheduler.insert {
                event = "next",
                action = resume_main,
                action_source = "simplified scheduler.insert",
            }
            coroutine.yield()
            cloe.log("info", "it's nice that this works now, but unexpected")
        end)
        local ok, result = coroutine.resume(test.co)
        if not ok then
            error(result)
        end
    end
}

cloe.schedule {
    on = "time=0.1",
    run = "succeed",
}
