function SendUDPtext
{
    Param ([string] $EndPointAddr,
           [int] $Port,
           [string] $Message)

    # Loopback Address 
    # $targetAddress = [System.Net.IPAddress]::IPv6Loopback

    # Call 
    # SendUDPtext -EndpointAddr "192.168.1.18" -Port 8888 -Message "Hello World"

    $targetAddress = [System.Net.IPAddress]::Parse($EndPointAddr.ToString())
    $endpoint = new-object System.Net.IPEndPoint ($targetAddress,$Port)

    $udpclient = new-Object System.Net.Sockets.UdpClient    
    $b = [Text.Encoding]::ASCII.GetBytes($Message)
    $bytesSent = $udpclient.Send($b,$b.length,$endpoint)

    $udpclient.Close()
}