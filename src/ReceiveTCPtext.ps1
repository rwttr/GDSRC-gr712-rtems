Function ReceiveTCPtext 
{
    # Usage
    # Listen to TCP Message From Any IP at specific port
    # 1.    load file function into powershell :>. ./ReceiveTCPtext.ps1
    # 2.    Execute Function :>ReceiveTCPtext $Port
    #           Example. :>ReceiveTCPtext 8888
    
    Param ( [int] $Port
    ) 
    Process {
        Try { 
            # Set up endpoint and start listening
            $Endpoint = new-object System.Net.IPEndPoint([ipaddress]::any,$Port) 
            $Listener = new-object System.Net.Sockets.TcpListener $EndPoint
            $Listener.start() 
 
            # Wait for an incoming connection 
            $data = $Listener.AcceptTcpClient() 
        
            # Stream setup
            $stream = $data.GetStream() 
            $bytes = New-Object System.Byte[] 1024

            # Read data from stream and write it to host
            #   read stream byte and check not equal to zero
            while (($i = $stream.Read($bytes,0,$bytes.Length)) -ne 0){
                # Data encoded in ascii text 
                $EncodedText = New-Object System.Text.ASCIIEncoding
                $data = $EncodedText.GetString($bytes,0, $i)

                # Print data in Console
                Write-Output $data
            }
         
            # Close TCP connection and stop listening
            $stream.close()
            $Listener.stop()
        }
        Catch {
            "Receive Message failed with: `n" + $Error[0]
        }
    }
}