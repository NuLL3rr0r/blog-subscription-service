#  (The MIT License)
#
#  Copyright (c) 2016 Mohammad S. Babaei
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.


FIND_LIBRARY ( KVM_LIBRARY NAMES kvm PATHS /usr/lib /usr/local/lib )

IF ( KVM_LIBRARY )
    SET ( KVM_FOUND TRUE )
ENDIF ( KVM_LIBRARY )


IF ( KVM_FOUND )
    MESSAGE ( STATUS "Found kvm library: ${KVM_LIBRARY}" )
ELSE ( KVM_FOUND )
    IF ( KVM_FIND_REQUIRED )
        MESSAGE ( FATAL_ERROR "Could not find kvm." )
    ENDIF ( KVM_FIND_REQUIRED )
ENDIF ( KVM_FOUND )
