function SendUDPtext
{
    # Usage
    # Send UDP Message to Endpoint IPAddress at specific port
    # 1.    load file function into powershell (:> shell symbol) :>. ./SendUDPtext.ps1
    # 2.    Execute Function :>SendUDPtext $ipAddress $Port "Message"
    #           Example. :>SendUDPtext 127.0.0.1 8888 "test Message"


    Param ([string] $EndPointAddr,
           [int] $Port,
           [string] $Message)

    $targetAddress = [System.Net.IPAddress]::Parse($EndPointAddr.ToString())
    $endpoint = new-object System.Net.IPEndPoint ($targetAddress,$Port)

    $udpclient = new-Object System.Net.Sockets.UdpClient    
    $b = [Text.Encoding]::ASCII.GetBytes($Message)
    $bytesSent = $udpclient.Send($b,$b.length,$endpoint)

    $udpclient.Close()
}