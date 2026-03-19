import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Button } from '@/components/ui/button'
import { ShieldCheck, Key } from 'lucide-react'

export default function SecurityTab() {
  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2"><Key className="w-5 h-5 text-primary" /> Password</CardTitle>
          <p className="text-sm text-muted-foreground">Change your password here. After saving, you'll be logged out.</p>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="space-y-2">
            <label className="text-sm font-medium leading-none">Current password</label>
            <input
              type="password"
              className="flex h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-sm shadow-sm transition-colors focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50"
              placeholder="••••••••"
            />
          </div>
          <div className="space-y-2">
            <label className="text-sm font-medium leading-none">New password</label>
            <input
              type="password"
              className="flex h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-sm shadow-sm transition-colors focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50"
              placeholder="••••••••"
            />
          </div>
          <div className="space-y-2">
            <label className="text-sm font-medium leading-none">Confirm new password</label>
            <input
              type="password"
              className="flex h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-sm shadow-sm transition-colors focus-visible:outline-none focus-visible:ring-1 focus-visible:ring-primary/50"
              placeholder="••••••••"
            />
          </div>
        </CardContent>
        <div className="flex items-center p-6 pt-0">
            <Button>Save password</Button>
        </div>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2"><ShieldCheck className="w-5 h-5 text-emerald-500" /> Two-factor authentication</CardTitle>
          <p className="text-sm text-muted-foreground">Add an extra layer of security to your account.</p>
        </CardHeader>
        <CardContent>
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 rounded-lg border border-border/50 p-4">
            <div className="space-y-1">
              <p className="text-sm font-medium leading-none">Authenticator app</p>
              <p className="text-sm text-muted-foreground">Use an app like Google Authenticator or 1Password to generate one-time codes.</p>
            </div>
            <Button variant="outline" className="shrink-0">Enable</Button>
          </div>
        </CardContent>
      </Card>

      <Card className="border-destructive/20 border">
        <CardHeader>
          <CardTitle className="text-destructive">Danger Zone</CardTitle>
          <p className="text-sm text-muted-foreground">Permanently delete your account and all of your content.</p>
        </CardHeader>
        <CardContent>
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 rounded-lg bg-destructive/10 p-4 border border-destructive/20">
            <div className="space-y-1">
              <p className="text-sm font-medium text-destructive leading-none">Delete Account</p>
              <p className="text-sm text-foreground/80">Once you delete your account, there is no going back. Please be certain.</p>
            </div>
            <Button variant="destructive" className="shrink-0">Delete Account</Button>
          </div>
        </CardContent>
      </Card>
    </div>
  )
}
