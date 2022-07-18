// udp-connect
// Kyle J Burgess

using System;
using System.Runtime.InteropServices;

public class UdcServer : IDisposable
{
    public event Action<UInt32> onConnectionSuccess;

    public event Action<UInt32> onConnectionTimeout;

    public event Action<UInt32> onConnectionLost;

    public event Action<UInt32> onConnectionRegained;

    public event Action<AddressIPv4, UInt16, ArraySegment<byte>> onReceivedMessageIPv4;

    public event Action<AddressIPv6, UInt16, ArraySegment<byte>> onReceivedMessageIPv6;

    // Types of messages
    public enum MessageType : UInt32
    {
        // Unreliable packets are basic UDP packets
        // there is no guarantee that they arrive
        UDC_UNRELIABLE_MESSAGE = 0u,

        // Reliable packets are guaranteed to arrive in order
        // and arrive exactly once. If connection is lost, then the reliable
        // messages will wait to be sent until connection is regained. If the
        // connection is closed with udcDisconnect, then pending reliable messages
        // are cleared.
        UDC_RELIABLE_MESSAGE = 1u,
    };

    // Message signature
    [StructLayout(LayoutKind.Sequential, Size = 4), Serializable]
    public struct Signature
    {
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] bytes;
    };

    // IPv4 Address
    // Stored as 4 octets in the format [0].[1].[2].[3]
    [StructLayout(LayoutKind.Sequential, Size = 4), Serializable]
    public struct AddressIPv4
    {
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] bytes;
    };

    // IPv6 Address
    // Stored as 8 segments in the format [0]:[1]:[2]:[3]:[4]:[5]:[6]:[7]
    // where each segment is in network byte-order (Big Endian)
    [StructLayout(LayoutKind.Sequential, Size = 16), Serializable]
    public struct AddressIPv6
    {
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 8)]
        public UInt16[] segments;
    };

    public UdcServer(Signature signature, UInt16 portIPv6, UInt16 portIPv4, uint bufferSize = 2048)
    {
        m_buffer = new byte[bufferSize + udcGetMinimumBufferSize()];
        m_server = udcCreateServer(signature, portIPv6, portIPv4, m_buffer, (UInt32)m_buffer.Length, IntPtr.Zero);

        if (m_server == IntPtr.Zero)
        {
            throw new Exception("Failed to start server.");
        }
    }

    public void Dispose()
    {
        TryDispose();
        GC.SuppressFinalize(this);
    }

    public static bool TryParseAddressIPv4(string nodeName, string serviceName, out AddressIPv4 address, out UInt16 port)
    {
        return udcTryParseAddressIPv4(nodeName, serviceName, out address, out port);
    }

    public static bool TryParseAddressIPv6(string nodeName, string serviceName, out AddressIPv6 address, out UInt16 port)
    {
        return udcTryParseAddressIPv6(nodeName, serviceName, out address, out port);
    }

    public bool TryConnect(string nodeName, string serviceName, UInt32 timeout, out UInt32 endPointId)
    {
        return udcTryConnect(m_server, nodeName, serviceName, timeout, out endPointId);
    }

    public bool TryConnectIPv4(AddressIPv4 address, UInt16 port, UInt32 timeout, out UInt32 endPointId)
    {
        return udcTryConnectIPv4(m_server, address, port, timeout, out endPointId);
    }

    public bool TryConnectIPv6(AddressIPv6 address, UInt16 port, UInt32 timeout, out UInt32 endPointId)
    {
        return udcTryConnectIPv6(m_server, address, port, timeout, out endPointId);
    }

    public bool GetStatus(UInt32 endPointId, out UInt32 ping)
    {
        return udcGetStatus(m_server, endPointId, out ping);
    }

    public void Disconnect(UInt32 endPointId)
    {
        udcDisconnect(m_server, endPointId);
    }

    public void SendMessage(UInt32 endPointId, byte[] data, MessageType reliability)
    {
        udcSendMessage(m_server, endPointId, data, (UInt32)data.Length, reliability);
    }

    public void ProcessEvents()
    {
        while (true)
        {
            IntPtr evnt = udcProcessEvents(m_server);

            if (evnt == IntPtr.Zero)
            {
                return;
            }

            switch(udcGetEventType(evnt))
            {
                case UdcEventType.UDC_EVENT_CONNECTION_SUCCESS:
                {
                    if (udcGetResultConnectionEvent(evnt, out UInt32 endPointId))
                    {
                        onConnectionSuccess?.Invoke(endPointId);
                    }
                    break;
                }
                case UdcEventType.UDC_EVENT_CONNECTION_TIMEOUT:
                {
                    if (udcGetResultConnectionEvent(evnt, out UInt32 endPointId))
                    {
                        onConnectionTimeout?.Invoke(endPointId);
                    }
                    break;
                }
                case UdcEventType.UDC_EVENT_CONNECTION_LOST:
                {
                    if (udcGetResultConnectionEvent(evnt, out UInt32 endPointId))
                    {
                        onConnectionLost?.Invoke(endPointId);
                    }
                    break;
                }
                case UdcEventType.UDC_EVENT_CONNECTION_REGAINED:
                {
                    if (udcGetResultConnectionEvent(evnt, out UInt32 endPointId))
                    {
                        onConnectionRegained?.Invoke(endPointId);
                    }
                    break;
                }
                case UdcEventType.UDC_EVENT_RECEIVE_MESSAGE_IPV4:
                {
                    if (udcGetResultMessageIPv4Event(evnt, out AddressIPv4 address, out UInt16 port, out UInt32 index, out UInt32 size))
                    {
                        onReceivedMessageIPv4(address, port, new ArraySegment<byte>(m_buffer, (int)index, (int)size));
                    }
                    break;
                }
                case UdcEventType.UDC_EVENT_RECEIVE_MESSAGE_IPV6:
                {
                    if (udcGetResultMessageIPv6Event(evnt, out AddressIPv6 address, out UInt16 port, out UInt32 index, out UInt32 size))
                    {
                        onReceivedMessageIPv6(address, port, new ArraySegment<byte>(m_buffer, (int)index, (int)size));
                    }
                    break;
                }
            }
        }
    }

    ~UdcServer()
    {
        TryDispose();
    }

    protected virtual void TryDispose()
    {
        if (!m_disposed)
        {
            udcDeleteServer(m_server);
            m_disposed = true;
        }
    }

    protected IntPtr m_server;

    protected byte[] m_buffer;

    protected bool m_disposed = false;

    // -------------------------------------------
    // udp-connect C++ Library Methods and Structs
    // -------------------------------------------

    // Types of events
    protected enum UdcEventType : UInt32
    {
        UDC_EVENT_CONNECTION_SUCCESS = 0u,
        UDC_EVENT_CONNECTION_TIMEOUT = 1u,
        UDC_EVENT_CONNECTION_LOST = 2u,
        UDC_EVENT_CONNECTION_REGAINED  = 3u,
        UDC_EVENT_RECEIVE_MESSAGE_IPV4 = 4u,
        UDC_EVENT_RECEIVE_MESSAGE_IPV6 = 5u,
    };

    [DllImport("libudpconnect", EntryPoint = "udcGetMinimumBufferSize", CallingConvention = CallingConvention.Cdecl)]
    protected static extern UInt32 udcGetMinimumBufferSize();

    [DllImport("libudpconnect", EntryPoint = "udcCreateServer", CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr udcCreateServer(Signature signature, UInt16 portIPv6, UInt16 portIPv4, byte[] buffer, UInt32 size, string logFileName);

    [DllImport("libudpconnect", EntryPoint = "udcDeleteServer", CallingConvention = CallingConvention.Cdecl)]
    protected static extern void udcDeleteServer(IntPtr server);

    [DllImport("libudpconnect", EntryPoint = "udcTryParseAddressIPv4", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcTryParseAddressIPv4(string nodeName, string serviceName, out AddressIPv4 address, out UInt16 port);

    [DllImport("libudpconnect", EntryPoint = "udcTryParseAddressIPv6", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcTryParseAddressIPv6(string nodeName, string serviceName, out AddressIPv6 address, out UInt16 port);

    [DllImport("libudpconnect", EntryPoint = "udcTryConnect", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcTryConnect(IntPtr server, string nodeName, string serviceName, UInt32 timeout, out UInt32 endPointId);

    [DllImport("libudpconnect", EntryPoint = "udcTryConnectIPv4", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcTryConnectIPv4(IntPtr server, AddressIPv4 address, UInt16 port, UInt32 timeout, out UInt32 endPointId);

    [DllImport("libudpconnect", EntryPoint = "udcTryConnectIPv6", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcTryConnectIPv6(IntPtr server, AddressIPv6 address, UInt16 port, UInt32 timeout, out UInt32 endPointId);

    [DllImport("libudpconnect", EntryPoint = "udcGetStatus", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcGetStatus(IntPtr server, UInt32 endPointId, out UInt32 ping);

    [DllImport("libudpconnect", EntryPoint = "udcDisconnect", CallingConvention = CallingConvention.Cdecl)]
    protected static extern void udcDisconnect(IntPtr server, UInt32 endPointId);

    [DllImport("libudpconnect", EntryPoint = "udcSendMessage", CallingConvention = CallingConvention.Cdecl)]
    protected static extern void udcSendMessage(IntPtr server, UInt32 endPointId, byte[] data, UInt32 size, MessageType reliability);

    [DllImport("libudpconnect", EntryPoint = "udcProcessEvents", CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr udcProcessEvents(IntPtr server);

    [DllImport("libudpconnect", EntryPoint = "udcGetEventType", CallingConvention = CallingConvention.Cdecl)]
    protected static extern UdcEventType udcGetEventType(IntPtr evnt);

    [DllImport("libudpconnect", EntryPoint = "udcGetResultConnectionEvent", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcGetResultConnectionEvent(IntPtr evnt, out UInt32 endPointId);

    [DllImport("libudpconnect", EntryPoint = "udcGetResultMessageIPv4Event", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcGetResultMessageIPv4Event(IntPtr evnt, out AddressIPv4 address, out UInt16 port, out UInt32 msgIndex, out UInt32 msgSize);

    [DllImport("libudpconnect", EntryPoint = "udcGetResultMessageIPv6Event", CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool udcGetResultMessageIPv6Event(IntPtr evnt, out AddressIPv6 address, out UInt16 port, out UInt32 msgIndex, out UInt32 msgSize);
}
