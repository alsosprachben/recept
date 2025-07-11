#!/usr/bin/env python3

import http.server

class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()

if __name__ == "__main__":
    import sys
    from http.server import HTTPServer
    server_address = ('', 8000)
    if len(sys.argv) == 2:
        server_address = ('', int(sys.argv[1]))

    httpd = HTTPServer(server_address, CORSRequestHandler)
    print(f"Serving on port {server_address[1]} with CORS and cross-origin isolation headers")
    httpd.serve_forever()
