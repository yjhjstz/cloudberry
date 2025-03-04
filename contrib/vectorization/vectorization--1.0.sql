-- dummy types use shell_in/shell_out that will error
-- if typein/typeout is really called

-- Function: clearfile(boolean)
-- This function is named 'clearfile' which takes a boolean parameter.

-- The function is defined to return a boolean value.
-- The implementation of this function is linked to an external C library.
-- The library location is specified as '$libdir/vectorization' and the function name within that library is 'clearfile'.
-- The language used for this function is C.

-- Regarding the behavior based on the input boolean value:
-- If the input is false, it means non - mandatory check for cleaning all vectorization files.
-- In this case, if a file is in use (occupied), it will not be processed. Only the files that are not in use will be cleared.
-- If the input is true, it indicates a mandatory clean - up of all temporary files related to vectorization.

CREATE TYPE pg_ext_aux.stddev;

CREATE FUNCTION pg_ext_aux.stddev_type_in(cstring)
 RETURNS pg_ext_aux.stddev
 AS '$libdir/vectorization', 'vector_stddev_in'
 LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION pg_ext_aux.stddev_type_out(pg_ext_aux.stddev)
 RETURNS cstring
 AS '$libdir/vectorization', 'vector_stddev_out'
 LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION clearfile(boolean)
  RETURNS boolean
AS '$libdir/vectorization', 'clearfile'
LANGUAGE C STRICT VOLATILE;

CREATE TYPE pg_ext_aux.stddev(
 input = pg_ext_aux.stddev_type_in,
 output = pg_ext_aux.stddev_type_out,
 internallength = 8);

--
-- Dummy type used to convert arrow types
--

CREATE TYPE pg_ext_aux.arrow_avg_int_bytea AS (sum int, count int);

