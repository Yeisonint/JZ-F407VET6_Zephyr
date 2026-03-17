import zenoh, time

def listener(sample):
    print(f"Received {sample.kind} ('{sample.key_expr}': '{sample.payload.to_string()}')")

if __name__ == "__main__":
    conf = zenoh.Config()
    conf.insert_json5("connect/endpoints", '["tcp/127.0.0.1:7447"]')

    with zenoh.open(conf) as session:
        sub = session.declare_subscriber('example/zenoh-pico-pub', listener)
        time.sleep(60)