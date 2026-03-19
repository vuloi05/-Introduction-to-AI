import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'

export default function NotificationsTab() {
  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <Card>
        <CardHeader>
          <CardTitle>Email Notifications</CardTitle>
          <p className="text-sm text-muted-foreground">Choose what you want to be notified about via email.</p>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="flex items-center justify-between gap-4 rounded-lg border border-border/50 p-4">
            <div className="space-y-0.5">
              <label className="text-base font-medium">Communication emails</label>
              <p className="text-sm text-muted-foreground">Receive emails about your account activity based on your communication preferences.</p>
            </div>
            <button type="button" className="peer inline-flex h-5 w-9 shrink-0 cursor-pointer items-center rounded-full border-2 border-transparent transition-colors bg-muted focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary/50">
              <span className="pointer-events-none block h-4 w-4 rounded-full bg-background shadow-lg ring-0 transition-transform translate-x-0"></span>
            </button>
          </div>
          <div className="flex items-center justify-between gap-4 rounded-lg border border-border/50 p-4">
            <div className="space-y-0.5">
              <label className="text-base font-medium">Marketing emails</label>
              <p className="text-sm text-muted-foreground">Receive emails about new products, features, and more.</p>
            </div>
            <button type="button" className="peer inline-flex h-5 w-9 shrink-0 cursor-pointer items-center rounded-full border-2 border-transparent transition-colors bg-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary/50">
              <span className="pointer-events-none block h-4 w-4 rounded-full bg-background shadow-[0_0_5px_rgba(0,0,0,0.2)] ring-0 transition-transform translate-x-4"></span>
            </button>
          </div>
          <div className="flex items-center justify-between gap-4 rounded-lg border border-border/50 p-4">
            <div className="space-y-0.5">
              <label className="text-base font-medium">Security emails</label>
              <p className="text-sm text-muted-foreground">Receive emails about suspicious login attempts and security alerts.</p>
            </div>
            <button type="button" className="peer inline-flex h-5 w-9 shrink-0 cursor-pointer items-center rounded-full border-2 border-transparent opacity-50 cursor-not-allowed bg-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary/50">
              <span className="pointer-events-none block h-4 w-4 rounded-full bg-background shadow-[0_0_5px_rgba(0,0,0,0.2)] ring-0 transition-transform translate-x-4"></span>
            </button>
          </div>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle>Push Notifications</CardTitle>
          <p className="text-sm text-muted-foreground">Configure desktop and in-app notifications.</p>
        </CardHeader>
        <CardContent className="space-y-4">
           <div className="flex items-center justify-between gap-4 rounded-lg border border-border/50 p-4">
            <div className="space-y-0.5">
              <label className="text-base font-medium">Desktop Push</label>
              <p className="text-sm text-muted-foreground">Allow notifications sent directly to your web browser or desktop.</p>
            </div>
            <button type="button" className="peer inline-flex h-5 w-9 shrink-0 cursor-pointer items-center rounded-full border-2 border-transparent transition-colors bg-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary/50">
              <span className="pointer-events-none block h-4 w-4 rounded-full bg-background shadow-[0_0_5px_rgba(0,0,0,0.2)] ring-0 transition-transform translate-x-4"></span>
            </button>
          </div>
          <div className="flex items-center justify-between gap-4 rounded-lg border border-border/50 p-4">
            <div className="space-y-0.5">
              <label className="text-base font-medium">Mentions</label>
              <p className="text-sm text-muted-foreground">Notify me when someone mentions me in a comment or ticket.</p>
            </div>
            <button type="button" className="peer inline-flex h-5 w-9 shrink-0 cursor-pointer items-center rounded-full border-2 border-transparent transition-colors bg-primary focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-primary/50">
              <span className="pointer-events-none block h-4 w-4 rounded-full bg-background shadow-[0_0_5px_rgba(0,0,0,0.2)] ring-0 transition-transform translate-x-4"></span>
            </button>
          </div>
        </CardContent>
      </Card>
    </div>
  )
}
