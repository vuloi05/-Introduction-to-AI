import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Button } from '@/components/ui/button'

export default function AppearanceTab() {
  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <Card>
        <CardHeader>
          <CardTitle>Appearance</CardTitle>
          <p className="text-sm text-muted-foreground">Customize the layout and theme of the dashboard.</p>
        </CardHeader>
        <CardContent className="space-y-6">
          <div className="space-y-4">
            <div>
              <h4 className="text-sm font-medium">Theme</h4>
              <p className="text-sm text-muted-foreground mt-1">Select the theme for the dashboard interface.</p>
            </div>
            <div className="grid grid-cols-2 md:grid-cols-3 gap-4">
              <div className="space-y-2 cursor-pointer group">
                <div className="h-[100px] w-full rounded-md border-2 border-primary bg-[#ecedef] p-2 transition-colors">
                  <div className="space-y-2 rounded-sm bg-white p-2 shadow-sm border h-full">
                    <div className="space-y-2 rounded-md bg-[#ecedef] p-2">
                      <div className="h-2 w-[80px] rounded-lg bg-white"></div>
                      <div className="h-2 w-[60px] rounded-lg bg-white"></div>
                    </div>
                    <div className="flex items-center space-x-2 rounded-md bg-[#ecedef] p-2">
                      <div className="h-4 w-4 rounded-full bg-white"></div>
                      <div className="h-2 w-[50px] rounded-lg bg-white"></div>
                    </div>
                  </div>
                </div>
                <span className="block text-center text-sm font-medium">Light</span>
              </div>
              
              <div className="space-y-2 cursor-pointer group">
                <div className="h-[100px] w-full rounded-md border-2 border-border bg-zinc-950 p-2 transition-colors group-hover:border-primary">
                  <div className="space-y-2 rounded-sm bg-zinc-800 p-2 shadow-sm border border-zinc-700 h-full">
                    <div className="space-y-2 rounded-md bg-zinc-950 p-2">
                      <div className="h-2 w-[80px] rounded-lg bg-zinc-800"></div>
                      <div className="h-2 w-[60px] rounded-lg bg-zinc-800"></div>
                    </div>
                    <div className="flex items-center space-x-2 rounded-md bg-zinc-950 p-2">
                      <div className="h-4 w-4 rounded-full bg-zinc-800"></div>
                      <div className="h-2 w-[50px] rounded-lg bg-zinc-800"></div>
                    </div>
                  </div>
                </div>
                <span className="block text-center text-sm font-medium text-muted-foreground group-hover:text-foreground">Dark</span>
              </div>

              <div className="space-y-2 cursor-pointer group">
                <div className="h-[100px] w-full rounded-md border-2 border-border p-2 transition-colors flex overflow-hidden group-hover:border-primary bg-gradient-to-r from-[#ecedef] to-zinc-950">
                  <div className="w-1/2 h-full bg-white/60 rounded-l-sm border-r border-[#ecedef]/50 p-1 backdrop-blur-sm">
                     <div className="h-full w-full bg-white rounded-sm shadow-sm flex flex-col gap-1 p-1">
                       <div className="h-1 w-[20px] rounded-md bg-[#ecedef]"></div>
                     </div>
                  </div>
                  <div className="w-1/2 h-full bg-zinc-800/60 rounded-r-sm p-1 backdrop-blur-sm">
                     <div className="h-full w-full bg-zinc-800 rounded-sm shadow-sm border border-zinc-700 flex flex-col gap-1 p-1">
                       <div className="h-1 w-[20px] rounded-md bg-zinc-950"></div>
                     </div>
                  </div>
                </div>
                <span className="block text-center text-sm font-medium text-muted-foreground group-hover:text-foreground">System</span>
              </div>
            </div>
          </div>

          <div className="space-y-3 pt-6 border-t border-border/50">
             <div>
              <h4 className="text-sm font-medium">Typography</h4>
              <p className="text-sm text-muted-foreground mt-1">Select your preferred font for the interface.</p>
            </div>
            <select className="h-9 w-full sm:w-[250px] rounded-md border border-input bg-transparent px-3 py-1 text-sm shadow-sm focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50 text-foreground cursor-pointer">
              <option value="geist">Geist (Default)</option>
              <option value="inter">Inter</option>
              <option value="sans">System UI</option>
            </select>
          </div>
        </CardContent>
        <div className="flex items-center p-6 pt-0">
            <Button>Update preferences</Button>
        </div>
      </Card>
    </div>
  )
}
