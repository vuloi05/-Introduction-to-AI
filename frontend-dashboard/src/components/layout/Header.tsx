import { Search, Bell } from 'lucide-react'

export default function Header() {
  return (
    <header className="h-16 border-b bg-card/60 backdrop-blur-md flex items-center justify-between px-6 sticky top-0 z-10">
      <div className="flex items-center flex-1">
        <div className="relative w-full max-w-md hidden md:block">
          <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
          <input 
            type="text" 
            placeholder="Search everything..." 
            className="w-full bg-background border border-border rounded-full py-2 pl-10 pr-4 text-sm focus:outline-none focus:ring-2 focus:ring-primary/20 transition-all font-sans"
          />
        </div>
      </div>
      
      <div className="flex items-center space-x-4">
        <button className="relative p-2 text-foreground/70 hover:bg-accent hover:text-accent-foreground rounded-full transition-colors">
          <Bell className="w-5 h-5" />
          <span className="absolute top-1 right-1 w-2 h-2 bg-destructive rounded-full"></span>
        </button>
        <div className="w-9 h-9 rounded-full bg-gradient-to-tr from-primary to-accent border-2 border-background shadow-sm cursor-pointer" />
      </div>
    </header>
  )
}
