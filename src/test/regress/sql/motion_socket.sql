-- The following test checks if the correct number and type of sockets are
-- created for motion connections both on QD and QE backends for the same
-- gp_session_id. Additionally we check if the source address used for creating
-- the motion sockets is equal to gp_segment_configuration.address.


-- start_matchignore
-- m/^INFO:  Checking postgres backend postgres:*/
-- end_matchignore
CREATE FUNCTION check_motion_sockets()
    RETURNS VOID as $$
import psutil, socket

# Create a temporary table to create a gang
plpy.execute("CREATE TEMP TABLE motion_socket_force_create_gang(i int);")

# We expect different number of sockets to be created for different
# interconnect types
# UDP: See calls to setupUDPListeningSocket in InitMotionUDPIFC
# TCP/PROXY: See call to setupTCPListeningSocket in InitMotionTCP
res = plpy.execute("SELECT current_setting('gp_interconnect_type');", 1)
ic_type = res[0]['current_setting']
if ic_type in ['tcp', 'proxy']:
    expected_socket_count_per_segment = 1
    expected_socket_kind = socket.SocketKind.SOCK_STREAM
elif ic_type=='udpifc':
    expected_socket_count_per_segment = 2
    expected_socket_kind = socket.SocketKind.SOCK_DGRAM
else:
    plpy.error('Unrecognized gp_interconnect_type {}.'.format(ic_type))

# Since this test is run on a single physical host we assume that all segments
# have the same gp_segment_configuration.address
res = plpy.execute("SELECT address FROM gp_segment_configuration;", 1)
hostip = socket.gethostbyname(res[0]['address'])

res = plpy.execute("SELECT pid from gp_backend_info();")
pids_to_check = [r['pid'] for r in res]

for pid in pids_to_check:
    # We iterate through all backends related to current session
    motion_socket_count = 0
    process = psutil.Process(pid)
    plpy.info('Checking postgres backend {}'.format(process.cmdline()[0]))
    for conn in process.connections():
        if conn.type == expected_socket_kind and conn.raddr == () \
        and conn.laddr.ip == hostip:
            motion_socket_count += 1

    if motion_socket_count != expected_socket_count_per_segment:
        plpy.error('Expected {} motion sockets but found {}. '\
        'For backend process {}. connections= {}'\
        .format(expected_socket_count_per_segment, process,\
        motion_socket_count, process.connections()))


$$ LANGUAGE plpython3u;
SELECT check_motion_sockets();
