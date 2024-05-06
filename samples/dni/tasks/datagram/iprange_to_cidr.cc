/* iprange
 * Copyright (C) 2003 Gabriel L. Somlo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * To compile:
 *  on Linux:
 *   gcc -o iprange iprange.c -O2 -Wall
 *  on Solaris 8, Studio 8 CC:
 *   cc -xO5 -xarch=v8plusa -xdepend iprange.c -o iprange -lnsl -lresolv
 *
 * CHANGELOG:
 *  2004-10-16 Paul Townsend (alpha alpha beta at purdue dot edu)
 *   - more general input/output formatting
 *
 */

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <vector>

#include "dni/tasks/datagram/cidr.h"
#include "spdlog/spdlog.h"

int main()
{
        spdlog::set_level(spdlog::level::trace);

        {
                dni::CIDR from = {0x01020000, 24};
                dni::CIDR to = {0x01020300, 24};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020000, 24};
                dni::CIDR to = {0x01020700, 24};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020000, 24};
                dni::CIDR to = {0x0102FF00, 24};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020000, 24};
                dni::CIDR to = {0x0102FE00, 24};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020300, 32};
                dni::CIDR to = {0x010203FF, 32};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020301, 32};
                dni::CIDR to = {0x010203FF, 32};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020301, 32};
                dni::CIDR to = {0x01020302, 32};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        printf("\n\n");

        {
                dni::CIDR from = {0x01020301, 33};
                dni::CIDR to = {0x01020302, 32};

                std::vector<dni::CIDR> ret = iprange_to_cidrs(from, to);
                for (auto&& cidr : ret)
                {
                        SPDLOG_DEBUG("{}", cidr);
                }
        }

        return 0;
}
