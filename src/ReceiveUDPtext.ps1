function ReceiveUDPtext
{

    # Usage
    # Listen to UDP Message From Any IP at specific port
    # 1.    load file function into powershell :>. ./ReceiveUDPtext.ps1
    # 2.    Execute Function :>ReceiveUDPtext $Port
    #           Example. :>ReceiveUDPtext 8888

    Param ([int] $Port)
    try
    {   
        # check Port is Available
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

    # Loop for listen to data; ESC to exit
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
            # Receive data via udp port
            $content = $udpclient.Receive( [ref]$endpoint )
            Write-Host "$($endpoint.Address.IPAddressToString):$($endpoint.Port) $([Text.Encoding]::ASCII.GetString($content))"
        }
    }

    # close udp port
    $udpclient.Close( )

}