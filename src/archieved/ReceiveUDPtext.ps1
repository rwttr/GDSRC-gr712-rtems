function ReceiveUDPtext
{
    Param ([int] $Port)
    try
    {
        $endpoint   = new-object System.Net.IPEndPoint ([IPAddress]::Any,$Port)
        $udpclient  = new-Object System.Net.Sockets.UdpClient $Port
    }
    catch
    {
        throw $_
        exit -1
    }    
    Write-Host "Press ESC to stop the udp listener ..." -fore yellow
    Write-Host ""
    while( $true )
    {
        if( $host.ui.RawUi.KeyAvailable )
        {
            $key = $host.ui.RawUI.ReadKey( "NoEcho,IncludeKeyUp,IncludeKeyDown" )
            if( $key.VirtualKeyCode -eq 27 )
            {   break   }
        }

        if( $udpclient.Available )
        {
            $content = $udpclient.Receive( [ref]$endpoint )
            Write-Host "$($endpoint.Address.IPAddressToString):$($endpoint.Port) $([Text.Encoding]::ASCII.GetString($content))"
        }
    }

    $udpclient.Close( )

}