function SendTCPtext
{ 
    # Usage
    # Send TCP Message to Endpoint IPAddress at specific port
    # 1.    load file function into powershell (:> shell symbol) :>. ./SendTCPtext.ps1
    # 2.    Execute Function :>SendTCPtext $ipAddress $Port "Message"
    #           Example. :>SendTCPtext 127.0.0.1 8888 "test Message"

    Param ( [string] $EndPoint,
            [int] $Port, 
            [string] $Message
    )

    # Setup connection 
    $IP = [System.Net.Dns]::GetHostAddresses($EndPoint) 
    $Address = [System.Net.IPAddress]::Parse($IP) 
    $Socket = New-Object System.Net.Sockets.TCPClient($Address,$Port) 
        
    # Setup stream wrtier 
    $Stream = $Socket.GetStream() 
    $Writer = New-Object System.IO.StreamWriter($Stream)

    # Write message to stream
    $Message | % {
        $Writer.WriteLine($_)
        $Writer.Flush()
    }
        
    # Close connection and stream
    $Stream.Close()
    $Socket.Close()

}