-- Lualian application
-- for use with Elua

local lualian = require("lualian")
local  getopt = require("getopt")

local include_dirs = {}
local output_files = {}

local libname, modname, cprefix = nil, nil, ""

local printv = function() end

local quit = false
local args

getopt.parse {
    usage = "Usage: %prog [OPTIONS] file1.eo file2.eo ... fileN.eo",
    args  = arg,
    descs = {
        { category = "General" },

        { "h", "help", nil, help = "Show this message.", metavar = "CATEGORY",
            callback = function(d, parser, v)
                getopt.help(parser, v, io.stdout)
                quit = true
            end
        },
        { "v", "verbose", false, help = "Be verbose.",
            callback = function()
                printv = print
            end
        },

        { category = "Generator" },

        { "I", "include", true, help = "Include a directory.", metavar = "DIR",
            callback = function(d, p, v)
                include_dirs[#include_dirs + 1] = v
            end
        },
        { "L", "library", true, help = "Specify a C library name.",
            callback = function(d, p, v)
                libname = v
            end
        },
        { "M", "module", true, help = "Specify a module name.",
            callback = function(d, p, v)
                modname = v
            end
        },
        { "P", "prefix", true, help = "Specify a class name prefix "
            .. "to strip out for public interfaces.",
            callback = function(d, p, v)
                cprefix = v
            end
        },
        { "o", "output", true, help = "Specify output file name(s), by "
            .. "default goes to stdout.",
            callback = function(d, p, v)
                output_files[#output_files + 1] = v
            end
        }
    },
    error_cb = function(parser, msg)
        io.stderr:write(msg, "\n")
        getopt.help(parser, io.stderr)
        quit = true
    end,
    done_cb = function(parser, opts, argsv)
        if not libname and not quit then
            io.stderr:write("library name not specified\n")
            getopt.help(parser, io.stderr)
            quit = true
        end
        args = argsv
    end
}

if quit then return true end

for i, v in ipairs(include_dirs) do
    lualian.include_dir(v)
end

for i, fname in ipairs(args) do
    printv("Generating for file: " .. fname)
    local fstream = io.stdout
    if output_files[i] then
        printv("  Output file: " .. output_files[i])
        fstream = io.open(output_files[i], "w")
        if not fstream then
            error("Cannot open output file: " .. output_files)
        end
    else
        printv("  Output file: printing to stdout...")
    end
    lualian.generate(fname, modname, libname, cprefix, fstream)
end

return true