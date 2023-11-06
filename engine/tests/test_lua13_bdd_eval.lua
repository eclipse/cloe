local cloe = require("cloe")

do
    local project = require("project")
    project.configure_all({
        with_server = false,
    })
    project.init_report(require("report_config"), { foo = "bar" })
    project.set_realtime_factor(-1)
end

-- Example schedule_test with testing using the Lust library:
--
--     https://github.com/bjornbytes/lust
--
-- Inside the run function we need to use z:describe to start
-- using the Lust library, and we need to import `lust.it` and
-- `lust.expect` from it.
cloe.schedule_test {
    id = "d7f31aaa-ccab-421b-a9ae-06aa3835018b",
    on = "start",
    info = { hello = "test", rqm = "big number" },
    desc = "this is a long text for test",
    terminate = false,
    run = function(z, sync)
        -- If we want to use the Lust
        local lust = require("lust")
        local it, expect = lust.it, lust.expect

        cloe.log("info", "Entering test")

        z:describe("test group 0", function()
            it("", function()
                expect(true).to.be.truthy()
            end)
        end)

        cloe.log("info", "Asserting something...")

        z:describe("test group 1", function()
            it("time at start is 0s", function()
                expect(sync:time():s()).to.be(0)
            end)
        end)

        cloe.log("info", "Waiting 1s...")
        z:wait_duration("1s")

        z:describe("test group 2", function()
            it("yield does not work and the time has not advanced", function()
                expect(sync:time() >= cloe.Duration.new("1s")).to.be.truthy()
            end)
            it("time has advanced the wrong amount", function()
                expect(sync:time() == cloe.Duration.new("1s")).to.be.truthy()
            end)
        end)

        cloe.log("info", "We're good here.")
        z:succeed()
    end,
}
