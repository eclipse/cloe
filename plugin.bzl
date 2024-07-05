CLOE_PLUGINS = [
    "//plugins/basic",
    "//plugins/clothoid_fit",
    # "//plugins/esmini",
    "//plugins/gndtruth_extractor",
    "//plugins/minimator",
    "//plugins/mocks:demo_stuck",
    "//plugins/mocks:demo_printer",
    "//plugins/noisy_sensor:noisy_object_sensor",
    "//plugins/noisy_sensor:noisy_lane_sensor",
    "//plugins/speedometer",
    "//plugins/virtue",
]

def cloe_plugin(
    name,
    copts = None,
    visibility = None,
    **kwargs,
):
    """
    Defines a cc_binary target for a cloe plugin.

    See: https://docs.bazel.build/versions/master/be/c-cpp.html#cc_binary

    Args:
        name: plugin name
        **kwargs: additional cc_binary args

    Defaults:
        copts: ["-fvisibility=hidden", "-fvisibility-inlines-hidden"]
        visibility: ["//visibility:public"]
        linkshared: True (forced)
    """

    # Set default for copts:
    if copts == None:
        copts = []
    copts += [
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
    ]

    # Set default for visibility:
    if visibility == None:
        visibility = ["//visibility:public"]

    # Check correct usage:
    if "linkshared" in kwargs:
        fail("cloe_plugin: linkshared must be True, do not set it yourself")

    # Create target:
    native.cc_binary(
        name = name,
        copts = copts,
        linkshared = True,
        visibility = visibility,
        **kwargs,
    )

def cloe_plugin_library(
    name,
    copts = None,
    **kwargs,
):
    """
    Defines a cc_library target for a cloe plugin.

    See: https://docs.bazel.build/versions/master/be/c-cpp.html#cc_library

    Args:
        name: plugin name
        **kwargs: additional cc_binary args

    Defaults:
        copts: ["-fvisibility=hidden", "-fvisibility-inlines-hidden"]
        alwayslink: True (forced)
    """

    # Set default for copts:
    if copts == None:
        copts = []
    copts += [
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
    ]

    # Check correct usage:
    if "alwayslink" in kwargs:
        fail("cloe_plugin_library: alwayslink must be True, do not set it yourself")

    # Create target:
    native.cc_library(
        name = name,
        copts = copts,
        alwayslink = True,
        **kwargs,
    )
