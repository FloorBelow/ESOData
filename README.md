# ESOData

ESOData is a set of components for accessing various aspects of The Elder
Scrolls Online game client data, including both the virtual filesystem
(physically represented by a set of mnf and dat files) and the client-side
database. For the database, two parsers are provided: a rather slow dynamic
parser that does not require recompilation when database layout changes, and
an efficient static one that does.

While ESOData-test is provided as an usage example, a more complete example
would be ESOBrowser.

This repository also includes ESOProjectedFilesystem - a Windows service that
can be used in order to access ESO "filesystem" under a Windows mountpoint,
using projected filesystem facility.

Please note that the database support in ESOData is rather incomplete: it only
supports database format of currently outdated client versions 5.2.11 -
5.2.13, and most of the field names are missing, and even these that aren't,
may be wrong.

# Building

ESOData may be built using normal CMake procedures, and is generally
intended to included into an outer project as a submodule or by any other
means.

Please note that ESOData uses git submodules, which should be retrieved
before building.

Also note that a copy of RAD Game Tools Granny 3D library is required in order
to build ESOData, and that library is *not* included in this repository. See
3rdparty/granny2/NOTE-GRANNY2 for detials.

# Licensing

ESOData is licensed under the terms of the MIT license (see LICENSE).

Note that ESOData also references MIT-licensed archiveparse and DirectXTex
as submodules and includes copies of CLI11 library by Henry Schreiner and
Snappy compression library by Google, both licensed under 3-clause BSD
license.
