import std.stdio;
import std.conv;
import std.string;
import core.thread;
import serial.device;

void main()
{
	auto receive = new ubyte[1024];
	size_t read = -1;
	auto command = cast(ubyte[])"get\n";
	SerialPort serial = new SerialPort("/dev/ttyACM0", dur!("msecs")(1000), dur!("msecs")(1000));
	scope(exit) serial.close();
	serial.speed(BaudRate.BR_115200);
	Thread.sleep(dur!("msecs")(2000));
	while(true)
	{
		try
		{
			serial.write(command);
			Thread.sleep(dur!("msecs")(100));
			read = serial.read(receive);
			read.writeln;
			auto r = receive[0..read].assumeUTF.chomp;
			auto values = r.split("\r\n");
			foreach(v; values)
			{
				insert(v.to!string);
			}
			Thread.sleep(dur!("seconds")(5));
		}
		catch(TimeoutException e)
		{
		}
	}
}

void insert(string value)
{
	import std.json;
	import std.datetime.systime;
	import kaleidic.mongo_standalone;
	value.writeln;
	const(char)[] colName = "test.testcol";
	auto j = parseJSON(value);
	auto time = Clock.currTime();
	auto ts = UtcTimestamp(time.toUTC.toUnixTime * 1000);
	bson_value[] values;
	values ~= bson_value("time"     , ts);
	values ~= bson_value("temp"     , j["temp"].floating);
	values ~= bson_value("pressure" , j["pressure"].floating);
	values ~= bson_value("humidity" , j["humidity"].floating);
	values ~= bson_value("co2"      , j["co2"].integer);
	values ~= bson_value("co2status", j["co2status"].integer);
	values ~= bson_value("luminance", j["luminance"].integer);
	auto mongo = new MongoConnection("mongodb://localhost/test");
	scope(exit)mongo.socket.close();
	mongo.insert(false, colName, [document(values)]);
}

