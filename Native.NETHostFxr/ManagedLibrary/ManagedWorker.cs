using System;
using System.Runtime.InteropServices;

namespace ManagedLibrary
{
    public class ManagedWorker
    {
        // Marked as "UnmanagedCallersOnly" so that we don't have to specify a delegate
        // when getting a function pointer to it on the CPP side
        [UnmanagedCallersOnly]
        public static void Print()
        {
            Console.WriteLine("ManagedWorker.Print()");
        }
    }
}
