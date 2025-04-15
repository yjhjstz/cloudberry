package cn.cbdb.hivesync.kafka.prometheus;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;

import com.sun.net.httpserver.HttpServer;

import io.micrometer.prometheus.PrometheusMeterRegistry;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class PrometheusHTTPServer {
    private static final Logger log = LoggerFactory.getLogger(PrometheusHTTPServer.class);
    Thread http;

    public void startHttpServer(int port) {
        try {
            PrometheusMeterRegistry prometheusRegistry = HiveSyncMetrics.getPrometheusRegistry();
            HttpServer server =
                HttpServer.create(new InetSocketAddress(InetAddress.getLoopbackAddress(), port), 0);
            server.createContext("/prometheus", httpExchange -> {
                String response = prometheusRegistry.scrape();
                httpExchange.sendResponseHeaders(200, response.getBytes().length);
                try (OutputStream os = httpExchange.getResponseBody()) {
                    os.write(response.getBytes());
                }
            });

            log.info("Start prometheus http server and listen on " + server.getAddress());

            http = new Thread(server::start);
            http.setDaemon(true);
            http.start();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
