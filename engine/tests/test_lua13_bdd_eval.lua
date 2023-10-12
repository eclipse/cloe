local cloe = require("cloe")

do
    local project = require("project")
    project.configure_all {
        with_server = false,
    }
    project.init_report()
    project.set_realtime_factor(-1)
end

-- schedule_test with improved logging using LUST - https://github.com/bjornbytes/lust
cloe.schedule_test {
    -- Note that this is the same ID as used in BATS.
    id = "a3ac1491-0213-40cf-b227-0dbb503c96c9",
    on = "start",
    info = { hello = "test", rqm = "big number" },
    desc = "this is a long text for test",
    run = function(z, sync)
        local lust = require("lust")
        local it, expect = lust.it, lust.expect

        cloe.log("info", "Entering test")

        z.describe("test group 0", function()
            it("", function()
                expect(true).to.be.truthy()
            end)
        end)


        cloe.log("info", "Asserting something...")

        z.describe("test group 1", function()
            it("time at start is 0s", function()
                expect(sync:time():s()).to.be(0)
            end)
        end)

        cloe.log("info", "Waiting 1s...")
        z.wait_duration("1s")

        z.describe("test group 2", function()
            it("yield does not work and the time has not advanced", function()
                expect(sync:time() >= cloe.Duration.new("1s")).to.be.truthy()
            end)
            it("time has advanced the wrong amount", function()
                expect(sync:time() == cloe.Duration.new("1s")).to.be.truthy()
            end)
        end)

        cloe.log("info", "We're good here.")
        z.succeed()
    end
}
