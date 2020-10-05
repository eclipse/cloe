#!/bin/bash

# Set the colors for the terminal
cBOLD="\e[1m"
cESC="\e[0m"

cBLUE="\e[1;34m"
cRED="\e[1;31m"
cGREEN="\e[1;32m"
cWHITE="\e[1;37m"
cYELLOW="\e[1;33m"
cCYAN="\e[1;36m"
cMAGENTA="\e[1;35m"

# Usage: fatalf <TEXT>
# Prints:
#
#   Error: example text
# Exits with 1
fatalf() {
    errorf "$1" >&2
    exit 1
}

# Usage: errorf <TEXT>
# Prints:
#
#   Error: example text
errorf() {
    printf "$cBOLD${cRED}Error:$cWHITE $1 $cESC\n" >&2
}

errorff() {
    printf "$cBOLD${cRED}   -- $cWHITE $1 $cESC\n" >&2
}

# Usage: warnf <TEXT>
# Prints:
#
#   Warning: example text
warnf() {
    printf "$cBOLD${cYELLOW}Warning:$cWHITE $1 $cESC\n" >&2
}

warnff() {
    printf "$cBOLD${cYELLOW}     -- $cWHITE $1 $cESC\n" >&2
}

# Usage: infof <TEXT>
# Prints:
#
#   => example text
infof() {
    printf "$cBOLD$cGREEN=>$cWHITE $1 $cESC\n" >&2
}

infoff() {
    printf "$cBOLD$cGREEN--$cWHITE $1 $cESC\n" >&2
}

# Usage: stepf <TEXT>
# Prints:
#
#   -> example text
stepf() {
    printf "$cBOLD$cBLUE->$cWHITE $1 $cESC\n" >&2
}

stepff() {
    printf "$cBOLD$cBLUE--$cWHITE $1 $cESC\n" >&2
}

# Usage: confirm <PROMPT>
# Per default, the answer is false.
# Prints:
#
#   :: example prompt [y/N]
confirm() {
    local msg=$1
    printf "$cBOLD$cBLUE::$cWHITE $msg $cESC[y/N] " >&2
    local ans
    read -r -n1 ans
    echo
    case $ans in
        y|Y)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

# Usage: deny <PROMPT>
# Per default, the answer is false.
# Prints:
#
#   :: example prompt [Y/n]
deny() {
    local msg=$1
    printf "$cBOLD$cBLUE::$cWHITE $msg $cESC[Y/n] " >&2
    local ans
    read -r -n1 ans
    echo
    case $ans in
        n|N)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

confirm_Y() {
    ! deny "$1"
}

confirm_N() {
    confirm "$1"
}
