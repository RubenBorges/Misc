#include <sdbus-c++/sdbus-c++.h>
#include <vector>
#include <string>

// Yeah, global variable is ugly, but this is just an example and we want to access
// the concatenator instance from within the concatenate method handler to be able
// to emit signals.
sdbus::IObject* g_concatenator{};

void concatenate(sdbus::MethodCall call)
{
    // Deserialize the collection of numbers from the message
    std::vector<int> numbers;
    call >> numbers;

    // Deserialize separator from the message
    std::string separator;
    call >> separator;

    // Return error if there are no numbers in the collection
    if (numbers.empty())
        throw sdbus::Error(sdbus::Error::Name{"org.sdbuscpp.Concatenator.Error"}, "No numbers provided");

    std::string result;
    for (auto number : numbers)
    {
        result += (result.empty() ? std::string() : separator) + std::to_string(number);
    }

    // Serialize resulting string to the reply and send the reply to the caller
    auto reply = call.createReply();
    reply << result;
    reply.send();

    // Emit 'concatenated' signal
    sdbus::InterfaceName interfaceName{"org.sdbuscpp.Concatenator"};
    sdbus::SignalName signalName{"concatenated"};
    auto signal = g_concatenator->createSignal(interfaceName, signalName);
    signal << result;
    g_concatenator->emitSignal(signal);
}

int main(int argc, char *argv[])
{
    // Create D-Bus connection to (either the session or system) bus and requests a well-known name on it.
    sdbus::ServiceName serviceName{"org.sdbuscpp.concatenator"};
    auto connection = sdbus::createBusConnection(serviceName);

    // Create concatenator D-Bus object.
    sdbus::ObjectPath objectPath{"/org/sdbuscpp/concatenator"};
    auto concatenator = sdbus::createObject(*connection, std::move(objectPath));

    g_concatenator = concatenator.get();

    // Register D-Bus methods and signals on the concatenator object, and exports the object.
    sdbus::InterfaceName interfaceName{"org.sdbuscpp.Concatenator"};
    concatenator->addVTable( sdbus::MethodVTableItem{sdbus::MethodName{"concatenate"}, sdbus::Signature{"ais"}, {}, sdbus::Signature{"s"}, {}, &concatenate, {}}
                           , sdbus::SignalVTableItem{sdbus::MethodName{"concatenated"}, sdbus::Signature{"s"}, {}, {}} )
                           .forInterface(interfaceName);

    // Run the I/O event loop on the bus connection.
    connection->enterEventLoop();
}