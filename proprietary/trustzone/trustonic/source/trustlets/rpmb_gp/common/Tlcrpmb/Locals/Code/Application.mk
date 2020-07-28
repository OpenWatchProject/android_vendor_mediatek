# =============================================================================
#
# Main build file defining the project modules and their global variables.
#
# =============================================================================

# Don't remove this - mandatory
APP_PROJECT_PATH := $(call my-dir)

# The only STL implementation currently working with exceptions
APP_STL := gnustl_static

# Don't optimize for better debugging
APP_OPTIM := debug
